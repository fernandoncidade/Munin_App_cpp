#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <stop_token>
#include <string_view>
#include <thread>
#include <vector>

namespace Munin::Engine {

struct LineSpan {
    uint64_t offset{0};
    uint32_t length{0}; // Length excluding trailing \r\n

    [[nodiscard]] bool isValid() const noexcept { return length > 0 || offset > 0; }
};

class ChunkIndex {
public:
    static constexpr uint64_t LINES_PER_CHUNK = 1000;

    ChunkIndex();
    ~ChunkIndex();

    // Reset index state
    void clear();

    // Starts background indexing on mapped memory
    void startIndexing(
        const char* data,
        uint64_t size,
        std::function<void(uint64_t totalLines, float progress)> onProgress = nullptr,
        std::function<void(uint64_t finalTotalLines)> onFinished = nullptr
    );

    // Stops indexing thread if running
    void stopIndexing();

    // Appends new data range for Live Tail
    void appendRange(
        const char* data,
        uint64_t oldSize,
        uint64_t newSize,
        std::function<void(uint64_t totalLines)> onUpdated = nullptr
    );

    // Queries the LineSpan for a given 0-indexed line
    [[nodiscard]] LineSpan getLineSpan(const char* data, uint64_t fileSize, uint64_t lineIndex) const;

    // Direct string_view query
    [[nodiscard]] std::string_view getLine(const char* data, uint64_t fileSize, uint64_t lineIndex) const;

    [[nodiscard]] uint64_t totalLines() const noexcept { return m_totalLines.load(std::memory_order_relaxed); }
    [[nodiscard]] bool isIndexing() const noexcept { return m_isIndexing.load(std::memory_order_relaxed); }
    [[nodiscard]] bool isFinished() const noexcept { return m_isFinished.load(std::memory_order_relaxed); }
    [[nodiscard]] float progress() const noexcept { return m_progress.load(std::memory_order_relaxed); }
    [[nodiscard]] double speedMBs() const noexcept { return m_speedMBs.load(std::memory_order_relaxed); }
    [[nodiscard]] uint64_t chunkCount() const;

private:
    void runIndexing(std::stop_token stopToken, const char* data, uint64_t size);

    struct CacheEntry {
        uint64_t lineIndex{UINT64_MAX};
        LineSpan span{};
    };

    static constexpr size_t CACHE_SIZE = 4096;
    mutable std::vector<CacheEntry> m_lineCache;

    // Chunk 0 starts at offset 0 (line 0)
    // Chunk k starts at offset m_chunkOffsets[k] (line k * LINES_PER_CHUNK)
    std::vector<uint64_t> m_chunkOffsets;
    mutable std::shared_mutex m_chunksMutex;

    std::atomic<uint64_t> m_totalLines{0};
    std::atomic<bool> m_isIndexing{false};
    std::atomic<bool> m_isFinished{false};
    std::atomic<float> m_progress{0.0f};
    std::atomic<double> m_speedMBs{0.0};

    std::jthread m_workerThread;
    std::function<void(uint64_t, float)> m_onProgress;
    std::function<void(uint64_t)> m_onFinished;
};

} // namespace Munin::Engine
