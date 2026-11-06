#include "TimeSliceScanner.hpp"
#include "ChunkIndex.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace Munin::Engine {

std::optional<int64_t> TimeSliceScanner::parseTimestamp(std::string_view line) noexcept {
    if (line.size() < 19) return std::nullopt;

    size_t start = 0;
    if (line[0] == '[' || line[0] == '{' || line[0] == '"') {
        start = 1;
    }

    if (line.size() < start + 19) return std::nullopt;

    // Check pattern YYYY-MM-DD[T or space]HH:MM:SS
    auto isDigit = [](char c) { return c >= '0' && c <= '9'; };

    if (!isDigit(line[start + 0]) || !isDigit(line[start + 1]) ||
        !isDigit(line[start + 2]) || !isDigit(line[start + 3]) ||
        line[start + 4] != '-' ||
        !isDigit(line[start + 5]) || !isDigit(line[start + 6]) ||
        line[start + 7] != '-' ||
        !isDigit(line[start + 8]) || !isDigit(line[start + 9])) {
        return std::nullopt;
    }

    char sep = line[start + 10];
    if (sep != 'T' && sep != ' ' && sep != '_') {
        return std::nullopt;
    }

    if (!isDigit(line[start + 11]) || !isDigit(line[start + 12]) ||
        line[start + 13] != ':' ||
        !isDigit(line[start + 14]) || !isDigit(line[start + 15]) ||
        line[start + 16] != ':' ||
        !isDigit(line[start + 17]) || !isDigit(line[start + 18])) {
        return std::nullopt;
    }

    int year = (line[start + 0] - '0') * 1000 + (line[start + 1] - '0') * 100 + (line[start + 2] - '0') * 10 + (line[start + 3] - '0');
    unsigned month = (line[start + 5] - '0') * 10 + (line[start + 6] - '0');
    unsigned day = (line[start + 8] - '0') * 10 + (line[start + 9] - '0');
    int hour = (line[start + 11] - '0') * 10 + (line[start + 12] - '0');
    int min = (line[start + 14] - '0') * 10 + (line[start + 15] - '0');
    int sec = (line[start + 17] - '0') * 10 + (line[start + 18] - '0');

    if (month < 1 || month > 12 || day < 1 || day > 31 || hour > 23 || min > 59 || sec > 59) {
        return std::nullopt;
    }

    using namespace std::chrono;
    auto ymd = year_month_day{std::chrono::year{year}, std::chrono::month{month}, std::chrono::day{day}};
    if (!ymd.ok()) {
        return std::nullopt;
    }

    auto tp = sys_days{ymd} + hours{hour} + minutes{min} + seconds{sec};
    return duration_cast<seconds>(tp.time_since_epoch()).count();
}

std::string TimeSliceScanner::formatEpoch(int64_t epochSeconds) {
    using namespace std::chrono;
    sys_seconds tp{seconds{epochSeconds}};
    auto days = floor<std::chrono::days>(tp);
    year_month_day ymd{days};
    auto timeOfDay = tp - days;
    auto hrs = duration_cast<hours>(timeOfDay);
    auto mins = duration_cast<minutes>(timeOfDay - hrs);
    auto secs = duration_cast<seconds>(timeOfDay - hrs - mins);

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << static_cast<unsigned>(ymd.month()) << "/"
        << std::setw(2) << static_cast<unsigned>(ymd.day()) << " "
        << std::setw(2) << hrs.count() << ":"
        << std::setw(2) << mins.count() << ":"
        << std::setw(2) << secs.count();
    return oss.str();
}

TimeSliceResult TimeSliceScanner::buildDistribution(
    const char* data,
    uint64_t size,
    const ChunkIndex& index,
    size_t bucketCount,
    std::function<void(float)> onProgress
) {
    TimeSliceResult result;
    uint64_t totalLines = index.totalLines();
    if (!data || size == 0 || totalLines == 0 || bucketCount == 0) {
        return result;
    }

    // Step 1: Probe first and last available timestamps to find the time domain
    std::optional<int64_t> firstTime;
    std::optional<int64_t> lastTime;

    // Scan up to 500 lines from the start
    uint64_t probeHead = std::min<uint64_t>(500, totalLines);
    for (uint64_t i = 0; i < probeHead; ++i) {
        std::string_view line = index.getLine(data, size, i);
        auto t = parseTimestamp(line);
        if (t.has_value()) {
            firstTime = t;
            break;
        }
    }

    // Scan backwards from the end
    uint64_t probeTail = totalLines > 500 ? totalLines - 500 : 0;
    for (uint64_t i = totalLines; i > probeTail; --i) {
        std::string_view line = index.getLine(data, size, i - 1);
        auto t = parseTimestamp(line);
        if (t.has_value()) {
            lastTime = t;
            break;
        }
    }

    if (!firstTime.has_value() || !lastTime.has_value() || *lastTime <= *firstTime) {
        result.hasTimestamps = false;
        return result;
    }

    result.hasTimestamps = true;
    result.minEpoch = *firstTime;
    result.maxEpoch = *lastTime;

    int64_t totalDuration = result.maxEpoch - result.minEpoch;
    if (totalDuration <= 0) {
        totalDuration = 1;
    }

    result.buckets.resize(bucketCount);
    int64_t bucketDuration = (totalDuration + static_cast<int64_t>(bucketCount) - 1) / static_cast<int64_t>(bucketCount);
    if (bucketDuration <= 0) bucketDuration = 1;

    for (size_t b = 0; b < bucketCount; ++b) {
        result.buckets[b].epochStart = result.minEpoch + static_cast<int64_t>(b) * bucketDuration;
        result.buckets[b].epochEnd = result.buckets[b].epochStart + bucketDuration;
        result.buckets[b].labelStart = formatEpoch(result.buckets[b].epochStart);
        result.buckets[b].firstLineIndex = UINT64_MAX;
    }

    // Sample lines uniformly (up to 50,000 samples for instant sub-second response)
    uint64_t step = std::max<uint64_t>(1, totalLines / 50000);

    for (uint64_t lineIdx = 0; lineIdx < totalLines; lineIdx += step) {
        std::string_view line = index.getLine(data, size, lineIdx);
        auto tOpt = parseTimestamp(line);
        if (tOpt.has_value()) {
            int64_t t = *tOpt;
            if (t >= result.minEpoch && t <= result.maxEpoch) {
                size_t b = static_cast<size_t>((t - result.minEpoch) / bucketDuration);
                if (b >= bucketCount) b = bucketCount - 1;

                result.buckets[b].count += step;
                if (result.buckets[b].firstLineIndex == UINT64_MAX) {
                    result.buckets[b].firstLineIndex = lineIdx;
                }

                // Check for error keywords
                if (line.find("ERROR") != std::string_view::npos ||
                    line.find("FATAL") != std::string_view::npos ||
                    line.find("CRITICAL") != std::string_view::npos) {
                    result.buckets[b].errorCount += step;
                }
            }
        }
    }

    // Fix unassigned firstLineIndex by interpolating
    uint64_t lastKnownLine = 0;
    for (size_t b = 0; b < bucketCount; ++b) {
        if (result.buckets[b].firstLineIndex != UINT64_MAX) {
            lastKnownLine = result.buckets[b].firstLineIndex;
        } else {
            result.buckets[b].firstLineIndex = lastKnownLine;
        }
    }

    if (onProgress) {
        onProgress(1.0f);
    }

    return result;
}

} // namespace Munin::Engine
