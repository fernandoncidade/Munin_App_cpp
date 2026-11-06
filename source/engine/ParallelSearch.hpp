#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <regex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace Munin::Engine {

class ChunkIndex;

struct SearchMatch {
    uint64_t lineIndex{0};
    uint64_t byteOffset{0};
    uint32_t lineLength{0};
    uint32_t matchStartInLine{0};
    uint32_t matchLength{0};
};

struct SearchOptions {
    std::string query;
    bool isRegex{false};
    bool caseSensitive{false};
    bool invertMatch{false};
};

class ParallelSearch {
public:
    using MatchCallback = std::function<void(const std::vector<SearchMatch>& matches, float progress)>;
    using FinishedCallback = std::function<void(const std::vector<SearchMatch>& allMatches, double elapsedSeconds)>;

    ParallelSearch();
    ~ParallelSearch();

    // Starts an asynchronous search across the memory-mapped file
    void startSearch(
        const char* data,
        uint64_t size,
        const ChunkIndex& index,
        SearchOptions options,
        MatchCallback onMatches = nullptr,
        FinishedCallback onFinished = nullptr
    );

    // Cancels active search
    void cancel();

    [[nodiscard]] bool isSearching() const noexcept { return m_isSearching.load(std::memory_order_relaxed); }
    [[nodiscard]] float progress() const noexcept { return m_progress.load(std::memory_order_relaxed); }

private:
    void searchWorker(
        const char* data,
        uint64_t size,
        const ChunkIndex& index,
        SearchOptions options
    );

    std::atomic<bool> m_isSearching{false};
    std::atomic<bool> m_cancelRequested{false};
    std::atomic<float> m_progress{0.0f};

    std::jthread m_searchThread;
    MatchCallback m_onMatches;
    FinishedCallback m_onFinished;
};

} // namespace Munin::Engine
