#include "ParallelSearch.hpp"
#include "ChunkIndex.hpp"
#include "SimdUtils.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <future>
#include <mutex>
#include <regex>

namespace Munin::Engine {

namespace {

bool containsCaseInsensitive(std::string_view text, std::string_view target, size_t& matchPos) {
    if (target.empty()) {
        matchPos = 0;
        return true;
    }
    if (text.size() < target.size()) return false;

    auto it = std::search(
        text.begin(), text.end(),
        target.begin(), target.end(),
        [](char ch1, char ch2) {
            return std::tolower(static_cast<unsigned char>(ch1)) ==
                   std::tolower(static_cast<unsigned char>(ch2));
        }
    );

    if (it != text.end()) {
        matchPos = static_cast<size_t>(std::distance(text.begin(), it));
        return true;
    }
    return false;
}

} // anonymous namespace

ParallelSearch::ParallelSearch() = default;

ParallelSearch::~ParallelSearch() {
    cancel();
}

void ParallelSearch::cancel() {
    m_cancelRequested.store(true, std::memory_order_relaxed);
    if (m_searchThread.joinable()) {
        m_searchThread.join();
    }
    m_isSearching.store(false, std::memory_order_relaxed);
}

void ParallelSearch::startSearch(
    const char* data,
    uint64_t size,
    const ChunkIndex& index,
    SearchOptions options,
    MatchCallback onMatches,
    FinishedCallback onFinished
) {
    cancel();

    m_cancelRequested.store(false, std::memory_order_relaxed);
    m_progress.store(0.0f, std::memory_order_relaxed);
    m_isSearching.store(true, std::memory_order_relaxed);
    m_onMatches = std::move(onMatches);
    m_onFinished = std::move(onFinished);

    m_searchThread = std::jthread([this, data, size, &index, opts = std::move(options)]() {
        searchWorker(data, size, index, opts);
    });
}

void ParallelSearch::searchWorker(
    const char* data,
    uint64_t size,
    const ChunkIndex& index,
    SearchOptions options
) {
    using namespace std::chrono;
    auto startTime = high_resolution_clock::now();

    std::vector<SearchMatch> allMatches;
    uint64_t totalLines = index.totalLines();

    if (!data || size == 0 || totalLines == 0 || options.query.empty()) {
        m_isSearching.store(false, std::memory_order_relaxed);
        m_progress.store(1.0f, std::memory_order_relaxed);
        if (m_onFinished) {
            m_onFinished({}, 0.0);
        }
        return;
    }

    // Determine number of worker threads
    unsigned int numThreads = std::max(1u, std::thread::hardware_concurrency());
    uint64_t linesPerThread = (totalLines + numThreads - 1) / numThreads;

    std::unique_ptr<std::regex> compiledRegex;
    if (options.isRegex) {
        try {
            auto flags = std::regex_constants::ECMAScript | std::regex_constants::optimize;
            if (!options.caseSensitive) {
                flags |= std::regex_constants::icase;
            }
            compiledRegex = std::make_unique<std::regex>(options.query, flags);
        } catch (...) {
            // Invalid regex
            m_isSearching.store(false, std::memory_order_relaxed);
            if (m_onFinished) {
                m_onFinished({}, 0.0);
            }
            return;
        }
    }

    std::vector<std::vector<SearchMatch>> threadResults(numThreads);
    std::atomic<uint64_t> linesProcessed{0};

    std::vector<std::jthread> workers;
    workers.reserve(numThreads);

    for (unsigned int t = 0; t < numThreads; ++t) {
        uint64_t startLine = t * linesPerThread;
        uint64_t endLine = std::min(startLine + linesPerThread, totalLines);

        workers.emplace_back([&, t, startLine, endLine]() {
            std::vector<SearchMatch>& localMatches = threadResults[t];
            localMatches.reserve(1024);

            for (uint64_t lineIdx = startLine; lineIdx < endLine; ++lineIdx) {
                if (m_cancelRequested.load(std::memory_order_relaxed)) {
                    break;
                }

                LineSpan span = index.getLineSpan(data, size, lineIdx);
                if (span.length == 0 && span.offset == 0 && lineIdx >= totalLines) {
                    continue;
                }

                std::string_view lineStr(data + span.offset, span.length);
                bool matched = false;
                size_t matchPos = 0;
                size_t matchLen = 0;

                if (options.isRegex && compiledRegex) {
                    std::match_results<const char*> match;
                    const char* lineStart = data + span.offset;
                    const char* lineEnd = lineStart + span.length;

                    if (std::regex_search(lineStart, lineEnd, match, *compiledRegex)) {
                        matched = true;
                        matchPos = static_cast<size_t>(match.position());
                        matchLen = static_cast<size_t>(match.length());
                    }
                } else if (options.caseSensitive) {
                    size_t pos = lineStr.find(options.query);
                    if (pos != std::string_view::npos) {
                        matched = true;
                        matchPos = pos;
                        matchLen = options.query.size();
                    }
                } else {
                    if (containsCaseInsensitive(lineStr, options.query, matchPos)) {
                        matched = true;
                        matchLen = options.query.size();
                    }
                }

                if (options.invertMatch) {
                    matched = !matched;
                    if (matched) {
                        matchPos = 0;
                        matchLen = 0;
                    }
                }

                if (matched) {
                    localMatches.push_back({
                        lineIdx,
                        span.offset,
                        span.length,
                        static_cast<uint32_t>(matchPos),
                        static_cast<uint32_t>(matchLen)
                    });
                }

                linesProcessed.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // Monitor progress
    while (linesProcessed.load(std::memory_order_relaxed) < totalLines &&
           !m_cancelRequested.load(std::memory_order_relaxed)) {
        float prog = static_cast<float>(static_cast<double>(linesProcessed.load(std::memory_order_relaxed)) / static_cast<double>(totalLines));
        m_progress.store(prog, std::memory_order_relaxed);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Join workers
    for (auto& w : workers) {
        if (w.joinable()) {
            w.join();
        }
    }

    if (m_cancelRequested.load(std::memory_order_relaxed)) {
        m_isSearching.store(false, std::memory_order_relaxed);
        return;
    }

    // Merge sorted results
    size_t totalMatchCount = 0;
    for (const auto& res : threadResults) {
        totalMatchCount += res.size();
    }

    allMatches.reserve(totalMatchCount);
    for (auto& res : threadResults) {
        allMatches.insert(allMatches.end(),
                          std::make_move_iterator(res.begin()),
                          std::make_move_iterator(res.end()));
    }

    double elapsedSec = duration_cast<duration<double>>(high_resolution_clock::now() - startTime).count();

    m_progress.store(1.0f, std::memory_order_relaxed);
    m_isSearching.store(false, std::memory_order_relaxed);

    if (m_onMatches) {
        m_onMatches(allMatches, 1.0f);
    }
    if (m_onFinished) {
        m_onFinished(allMatches, elapsedSec);
    }
}

} // namespace Munin::Engine
