#include "ChunkIndex.hpp"
#include "SimdUtils.hpp"

#include <chrono>

namespace Munin::Engine {

ChunkIndex::ChunkIndex()
    : m_lineCache(CACHE_SIZE) {
}

ChunkIndex::~ChunkIndex() {
    stopIndexing();
}

void ChunkIndex::clear() {
    stopIndexing();

    {
        std::unique_lock lock(m_chunksMutex);
        m_chunkOffsets.clear();
        for (auto& entry : m_lineCache) {
            entry.lineIndex = UINT64_MAX;
            entry.span = {};
        }
    }

    m_totalLines.store(0, std::memory_order_relaxed);
    m_isIndexing.store(false, std::memory_order_relaxed);
    m_isFinished.store(false, std::memory_order_relaxed);
    m_progress.store(0.0f, std::memory_order_relaxed);
    m_speedMBs.store(0.0, std::memory_order_relaxed);
}

void ChunkIndex::stopIndexing() {
    if (m_workerThread.joinable()) {
        m_workerThread.request_stop();
        m_workerThread.join();
    }
    m_isIndexing.store(false, std::memory_order_relaxed);
}

uint64_t ChunkIndex::chunkCount() const {
    std::shared_lock lock(m_chunksMutex);
    return static_cast<uint64_t>(m_chunkOffsets.size());
}

void ChunkIndex::startIndexing(
    const char* data,
    uint64_t size,
    std::function<void(uint64_t, float)> onProgress,
    std::function<void(uint64_t)> onFinished
) {
    clear();

    if (!data || size == 0) {
        m_isFinished.store(true, std::memory_order_relaxed);
        m_progress.store(1.0f, std::memory_order_relaxed);
        if (onFinished) onFinished(0);
        return;
    }

    m_onProgress = std::move(onProgress);
    m_onFinished = std::move(onFinished);

    {
        std::unique_lock lock(m_chunksMutex);
        m_chunkOffsets.reserve(size / (LINES_PER_CHUNK * 40) + 16);
        m_chunkOffsets.push_back(0); // Line 0 starts at offset 0
    }

    m_isIndexing.store(true, std::memory_order_relaxed);
    m_isFinished.store(false, std::memory_order_relaxed);

    m_workerThread = std::jthread([this, data, size](std::stop_token stopToken) {
        runIndexing(stopToken, data, size);
    });
}

void ChunkIndex::runIndexing(std::stop_token stopToken, const char* data, uint64_t size) {
    using namespace std::chrono;
    auto startTime = high_resolution_clock::now();
    auto lastReportTime = startTime;

    uint64_t currentOffset = 0;
    uint64_t currentLines = 0;
    uint64_t linesSinceLastChunk = 0;

    // Scan loop
    constexpr size_t BLOCK_SCAN = 64 * 1024; // 64 KB blocks

    while (currentOffset < size && !stopToken.stop_requested()) {
        size_t bytesToScan = static_cast<size_t>(std::min<uint64_t>(BLOCK_SCAN, size - currentOffset));
        const char* blockPtr = data + currentOffset;

        size_t blockOffset = 0;
        while (blockOffset < bytesToScan && !stopToken.stop_requested()) {
            size_t nextNl = SimdUtils::findNextNewline(blockPtr, bytesToScan, blockOffset);
            if (nextNl < bytesToScan) {
                ++currentLines;
                ++linesSinceLastChunk;
                blockOffset = nextNl + 1;

                if (linesSinceLastChunk == LINES_PER_CHUNK) {
                    uint64_t nextChunkStart = currentOffset + blockOffset;
                    {
                        std::unique_lock lock(m_chunksMutex);
                        m_chunkOffsets.push_back(nextChunkStart);
                    }
                    linesSinceLastChunk = 0;
                    m_totalLines.store(currentLines, std::memory_order_relaxed);
                }
            } else {
                blockOffset = bytesToScan;
            }
        }

        currentOffset += bytesToScan;

        // Periodic progress reporting (every ~50ms)
        auto now = high_resolution_clock::now();
        if (duration_cast<milliseconds>(now - lastReportTime).count() >= 50) {
            float prog = static_cast<float>(static_cast<double>(currentOffset) / static_cast<double>(size));
            double elapsedSec = duration_cast<duration<double>>(now - startTime).count();
            double speed = elapsedSec > 0.001 ? (static_cast<double>(currentOffset) / (1024.0 * 1024.0)) / elapsedSec : 0.0;

            m_totalLines.store(currentLines, std::memory_order_relaxed);
            m_progress.store(prog, std::memory_order_relaxed);
            m_speedMBs.store(speed, std::memory_order_relaxed);

            if (m_onProgress) {
                m_onProgress(currentLines, prog);
            }
            lastReportTime = now;
        }
    }

    if (stopToken.stop_requested()) {
        m_isIndexing.store(false, std::memory_order_relaxed);
        return;
    }

    // If file has characters after the last newline, count the final line
    if (size > 0 && data[size - 1] != '\n') {
        ++currentLines;
    }

    m_totalLines.store(currentLines, std::memory_order_release);
    m_progress.store(1.0f, std::memory_order_relaxed);

    auto totalTime = duration_cast<duration<double>>(high_resolution_clock::now() - startTime).count();
    if (totalTime > 0.001) {
        m_speedMBs.store((static_cast<double>(size) / (1024.0 * 1024.0)) / totalTime, std::memory_order_relaxed);
    }

    m_isIndexing.store(false, std::memory_order_relaxed);
    m_isFinished.store(true, std::memory_order_release);

    if (m_onProgress) {
        m_onProgress(currentLines, 1.0f);
    }
    if (m_onFinished) {
        m_onFinished(currentLines);
    }
}

void ChunkIndex::appendRange(
    const char* data,
    uint64_t oldSize,
    uint64_t newSize,
    std::function<void(uint64_t)> onUpdated
) {
    if (!data || newSize <= oldSize) return;

    uint64_t currentOffset = oldSize;
    uint64_t currentLines = m_totalLines.load(std::memory_order_relaxed);

    // Calculate how many lines were in the incomplete final chunk
    uint64_t linesSinceLastChunk = currentLines % LINES_PER_CHUNK;

    while (currentOffset < newSize) {
        size_t nextNl = SimdUtils::findNextNewline(data, newSize, currentOffset);
        if (nextNl < newSize) {
            ++currentLines;
            ++linesSinceLastChunk;
            currentOffset = nextNl + 1;

            if (linesSinceLastChunk == LINES_PER_CHUNK) {
                {
                    std::unique_lock lock(m_chunksMutex);
                    m_chunkOffsets.push_back(currentOffset);
                }
                linesSinceLastChunk = 0;
            }
        } else {
            // Reached newSize without trailing newline
            currentOffset = newSize;
        }
    }

    m_totalLines.store(currentLines, std::memory_order_release);

    if (onUpdated) {
        onUpdated(currentLines);
    }
}

LineSpan ChunkIndex::getLineSpan(const char* data, uint64_t fileSize, uint64_t lineIndex) const {
    if (!data || fileSize == 0 || lineIndex >= m_totalLines.load(std::memory_order_acquire)) {
        return {};
    }

    // Check viewport LRU cache first
    size_t cacheIdx = lineIndex % CACHE_SIZE;
    if (m_lineCache[cacheIdx].lineIndex == lineIndex) {
        return m_lineCache[cacheIdx].span;
    }

    uint64_t chunkIdx = lineIndex / LINES_PER_CHUNK;
    uint64_t chunkStart = 0;
    uint64_t chunkEnd = fileSize;

    {
        std::shared_lock lock(m_chunksMutex);
        if (chunkIdx < m_chunkOffsets.size()) {
            chunkStart = m_chunkOffsets[chunkIdx];
            if (chunkIdx + 1 < m_chunkOffsets.size()) {
                chunkEnd = m_chunkOffsets[chunkIdx + 1];
            }
        } else {
            return {};
        }
    }

    uint64_t relLine = lineIndex % LINES_PER_CHUNK;
    uint64_t lineStart = chunkStart;

    if (relLine > 0) {
        size_t nthOffset = 0;
        if (SimdUtils::findNthNewline(data + chunkStart, chunkEnd - chunkStart, relLine - 1, nthOffset)) {
            lineStart = chunkStart + nthOffset + 1;
        } else {
            return {};
        }
    }

    if (lineStart >= fileSize) {
        return {};
    }

    size_t nextNl = SimdUtils::findNextNewline(data, fileSize, lineStart);
    uint64_t rawLength = nextNl - lineStart;

    // Strip carriage return '\r' if present
    if (rawLength > 0 && data[lineStart + rawLength - 1] == '\r') {
        --rawLength;
    }

    LineSpan span{lineStart, static_cast<uint32_t>(rawLength)};
    m_lineCache[cacheIdx] = {lineIndex, span};

    return span;
}

std::string_view ChunkIndex::getLine(const char* data, uint64_t fileSize, uint64_t lineIndex) const {
    LineSpan span = getLineSpan(data, fileSize, lineIndex);
    if (!span.isValid() && span.offset == 0 && span.length == 0 && lineIndex >= m_totalLines.load(std::memory_order_relaxed)) {
        return {};
    }
    return std::string_view(data + span.offset, span.length);
}

} // namespace Munin::Engine
