#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Munin::Engine {

class ChunkIndex;

struct TimeBucket {
    int64_t epochStart{0};
    int64_t epochEnd{0};
    std::string labelStart;
    uint64_t count{0};
    uint64_t errorCount{0};
    uint64_t firstLineIndex{0};
};

struct TimeSliceResult {
    bool hasTimestamps{false};
    int64_t minEpoch{0};
    int64_t maxEpoch{0};
    std::vector<TimeBucket> buckets;
};

class TimeSliceScanner {
public:
    // Fast parser for ISO-8601 or common log timestamps
    // Returns unix epoch seconds if timestamp was detected, or nullopt
    static std::optional<int64_t> parseTimestamp(std::string_view line) noexcept;

    // Formats epoch to UTC / local readable HH:MM or YYYY-MM-DD HH:MM
    static std::string formatEpoch(int64_t epochSeconds);

    // Scans the indexed log file to build histogram distribution
    static TimeSliceResult buildDistribution(
        const char* data,
        uint64_t size,
        const ChunkIndex& index,
        size_t bucketCount = 60,
        std::function<void(float progress)> onProgress = nullptr
    );
};

} // namespace Munin::Engine
