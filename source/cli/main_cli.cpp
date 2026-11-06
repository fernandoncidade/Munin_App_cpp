#include "engine/MemoryMappedFile.hpp"
#include "engine/ChunkIndex.hpp"
#include "engine/ParallelSearch.hpp"
#include "engine/LiveTailWatcher.hpp"
#include "engine/SimdUtils.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#endif

using namespace Munin::Engine;

static constexpr const char* APP_VERSION = "2026.11.6.0";

static void printBanner() {
    std::cout << "===============================================================\n";
    std::cout << "  Munin CLI — Massive Log File Inspector & Search Engine       \n";
    std::cout << "  Version " << APP_VERSION << " | C++20 Win32 SIMD Engine                  \n";
    std::cout << "===============================================================\n\n";
}

static void printHelp() {
    printBanner();
    std::cout << "Usage:\n";
    std::cout << "  munin-app-cli.exe <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  search <file> <term>     Search query in log file using multi-core SIMD engine\n";
    std::cout << "  info <file>              Display file size, line count, and indexing metrics\n";
    std::cout << "  tail <file>              Follow log file in real-time (live stream to console)\n";
    std::cout << "  bench [file]             Run indexing and search benchmarks\n";
    std::cout << "  --version, -v            Show official version\n";
    std::cout << "  --help, -h               Show this help message\n\n";
    std::cout << "Options for 'search':\n";
    std::cout << "  --format <text|json>     Output format (default: text)\n";
    std::cout << "  --ignore-case, -i        Case-insensitive search (default: false)\n";
    std::cout << "  --regex, -r              Treat search term as a regular expression\n";
    std::cout << "  --invert, -v             Invert match (show lines that do not match)\n";
    std::cout << "  --max <n>                Limit maximum reported matches (default: 1000)\n";
    std::cout << "  --no-banner              Suppress banner in text output\n\n";
    std::cout << "Examples:\n";
    std::cout << "  munin-app-cli.exe search \"C:\\Logs\\app.log\" \"ERROR\" --format text\n";
    std::cout << "  munin-app-cli.exe search \"C:\\Logs\\app.log\" \"UserLoginFailed\" -i --max 50\n";
    std::cout << "  munin-app-cli.exe info \"C:\\Logs\\production.log\"\n";
    std::cout << "  munin-app-cli.exe tail \"C:\\Logs\\live.log\"\n";
    std::cout << "  munin-app-cli.exe bench\n";
}

static std::wstring toWide(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
    if (size <= 0) {
        size = MultiByteToWideChar(CP_ACP, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
        if (size <= 0) return L"";
        std::wstring wstr(size, 0);
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), static_cast<int>(str.size()), &wstr[0], size);
        return wstr;
    }
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &wstr[0], size);
    return wstr;
}

static std::string escapeJson(std::string_view s) {
    std::string res;
    res.reserve(s.size() + 16);
    for (char c : s) {
        switch (c) {
            case '"':  res += "\\\""; break;
            case '\\': res += "\\\\"; break;
            case '\b': res += "\\b";  break;
            case '\f': res += "\\f";  break;
            case '\n': res += "\\n";  break;
            case '\r': res += "\\r";  break;
            case '\t': res += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    res += buf;
                } else {
                    res += c;
                }
                break;
        }
    }
    return res;
}

static int cmdInfo(const std::string& filePath) {
    std::wstring wpath = toWide(filePath);
    MemoryMappedFile mmap;
    if (!mmap.open(wpath)) {
        std::cerr << "Error opening file: " << mmap.lastError() << "\n";
        return 1;
    }

    double sizeMB = static_cast<double>(mmap.size()) / (1024.0 * 1024.0);

    ChunkIndex index;
    auto t0 = std::chrono::high_resolution_clock::now();
    index.startIndexing(mmap.data(), mmap.size());
    while (index.isIndexing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double elapsedSec = std::chrono::duration<double>(t1 - t0).count();

    std::cout << "File: " << filePath << "\n";
    std::cout << "Size: " << std::fixed << std::setprecision(2) << sizeMB << " MB (" << mmap.size() << " bytes)\n";
    std::cout << "Total Lines: " << index.totalLines() << "\n";
    std::cout << "Chunks: " << index.chunkCount() << " (" << ChunkIndex::LINES_PER_CHUNK << " lines/chunk)\n";
    std::cout << "Indexing Time: " << std::setprecision(4) << elapsedSec << " s\n";
    if (elapsedSec > 0.0001) {
        std::cout << "Throughput: " << std::setprecision(2) << (sizeMB / elapsedSec) << " MB/s\n";
    }

    return 0;
}

static int cmdSearch(const std::string& filePath, const std::string& query,
                     bool caseSensitive, bool isRegex, bool invert,
                     size_t maxMatches, const std::string& format, bool noBanner) {
    if (format != "json" && !noBanner) {
        printBanner();
    }

    std::wstring wpath = toWide(filePath);
    MemoryMappedFile mmap;
    if (!mmap.open(wpath)) {
        if (format == "json") {
            std::cout << "{\"error\": \"Failed to open file: " << escapeJson(mmap.lastError()) << "\"}\n";
        } else {
            std::cerr << "Error opening file: " << mmap.lastError() << "\n";
        }
        return 1;
    }

    ChunkIndex index;
    index.startIndexing(mmap.data(), mmap.size());
    while (index.isIndexing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    SearchOptions opt;
    opt.query = query;
    opt.caseSensitive = caseSensitive;
    opt.isRegex = isRegex;
    opt.invertMatch = invert;

    ParallelSearch searcher;
    std::vector<SearchMatch> allMatches;
    double searchElapsed = 0.0;
    bool done = false;

    auto tSearch0 = std::chrono::high_resolution_clock::now();
    searcher.startSearch(
        mmap.data(),
        mmap.size(),
        index,
        opt,
        nullptr,
        [&](const std::vector<SearchMatch>& matches, double elapsed) {
            allMatches = matches;
            searchElapsed = elapsed;
            done = true;
        }
    );

    while (!done && searcher.isSearching()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    auto tSearch1 = std::chrono::high_resolution_clock::now();
    if (searchElapsed <= 0.0) {
        searchElapsed = std::chrono::duration<double>(tSearch1 - tSearch0).count();
    }

    size_t count = allMatches.size();
    size_t displayCount = std::min(count, maxMatches);

    if (format == "json") {
        std::cout << "{\n";
        std::cout << "  \"file\": \"" << escapeJson(filePath) << "\",\n";
        std::cout << "  \"query\": \"" << escapeJson(query) << "\",\n";
        std::cout << "  \"totalLines\": " << index.totalLines() << ",\n";
        std::cout << "  \"matchesFound\": " << count << ",\n";
        std::cout << "  \"matchesReported\": " << displayCount << ",\n";
        std::cout << "  \"searchTimeSeconds\": " << std::fixed << std::setprecision(6) << searchElapsed << ",\n";
        std::cout << "  \"matches\": [\n";

        for (size_t i = 0; i < displayCount; ++i) {
            const auto& m = allMatches[i];
            std::string_view lineText = index.getLine(mmap.data(), mmap.size(), m.lineIndex);
            std::cout << "    {\n";
            std::cout << "      \"line\": " << (m.lineIndex + 1) << ",\n";
            std::cout << "      \"offset\": " << m.byteOffset << ",\n";
            std::cout << "      \"text\": \"" << escapeJson(lineText) << "\"\n";
            std::cout << "    }" << (i + 1 < displayCount ? "," : "") << "\n";
        }
        std::cout << "  ]\n";
        std::cout << "}\n";
    } else {
        std::cout << "Searching for: \"" << query << "\" in " << filePath << "\n";
        std::cout << "Total Lines: " << index.totalLines()
                  << " | Matches: " << count
                  << " | Time: " << std::fixed << std::setprecision(4) << searchElapsed << " s\n";
        std::cout << "---------------------------------------------------------------\n";

        for (size_t i = 0; i < displayCount; ++i) {
            const auto& m = allMatches[i];
            std::string_view line = index.getLine(mmap.data(), mmap.size(), m.lineIndex);
            std::cout << "[" << std::setw(7) << (m.lineIndex + 1) << "] " << line << "\n";
        }

        if (count > displayCount) {
            std::cout << "\n... (" << (count - displayCount) << " additional matches truncated, use --max to expand)\n";
        }
    }

    return 0;
}

static int cmdTail(const std::string& filePath) {
    printBanner();
    std::cout << "Tailing: " << filePath << " (Press Ctrl+C to stop)\n";
    std::cout << "---------------------------------------------------------------\n";

    std::wstring wpath = toWide(filePath);
    MemoryMappedFile mmap;
    if (!mmap.open(wpath)) {
        std::cerr << "Error opening file: " << mmap.lastError() << "\n";
        return 1;
    }

    uint64_t currentSize = mmap.size();

    // Show initial last 10 lines
    ChunkIndex index;
    index.startIndexing(mmap.data(), mmap.size());
    while (index.isIndexing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    uint64_t total = index.totalLines();
    uint64_t startLine = (total > 10) ? (total - 10) : 0;
    for (uint64_t l = startLine; l < total; ++l) {
        std::cout << index.getLine(mmap.data(), mmap.size(), l) << "\n";
    }

    // Polling loop for tail stream
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        WIN32_FILE_ATTRIBUTE_DATA fad{};
        if (GetFileAttributesExW(wpath.c_str(), GetFileExInfoStandard, &fad)) {
            ULARGE_INTEGER li;
            li.LowPart = fad.nFileSizeLow;
            li.HighPart = fad.nFileSizeHigh;
            uint64_t newSize = li.QuadPart;

            if (newSize > currentSize) {
                if (mmap.remap(newSize)) {
                    std::string_view appended = mmap.slice(currentSize, newSize - currentSize);
                    std::cout << appended;
                    std::cout.flush();
                    currentSize = newSize;
                }
            } else if (newSize < currentSize) {
                // File was truncated/rotated
                std::cout << "\n[!] File truncated or rotated. Reloading...\n";
                mmap.close();
                if (mmap.open(wpath)) {
                    currentSize = mmap.size();
                }
            }
        }
    }

    return 0;
}

static void generateSyntheticLog(const std::wstring& path, uint64_t lines) {
    FILE* fp = nullptr;
    _wfopen_s(&fp, path.c_str(), L"wb");
    if (!fp) return;
    setvbuf(fp, nullptr, _IOFBF, 2 * 1024 * 1024);

    const char* levels[] = {"INFO", "DEBUG", "WARN", "ERROR", "TRACE"};
    char buf[512];

    for (uint64_t i = 0; i < lines; ++i) {
        int h = static_cast<int>((i / 3600) % 24);
        int m = static_cast<int>((i / 60) % 60);
        int s = static_cast<int>(i % 60);
        int th = static_cast<int>(i % 8) + 1;
        const char* lvl = levels[i % 5];
        int uid = 1000 + static_cast<int>(i % 500);

        int len = snprintf(
            buf, sizeof(buf),
            "2026-10-17T%02d:%02d:%02d.123Z [worker-%02d] 10.0.0.%d [%s] Process completed - user_id=%d status=%d\n",
            h, m, s, th, (th * 23) % 254 + 1, lvl, uid, (lvl[0] == 'E' ? 500 : 200)
        );

        if (len > 0) {
            fwrite(buf, 1, len, fp);
        }
    }

    fclose(fp);
}

static int cmdBench(const std::string& customFile) {
    printBanner();
    std::wstring testFile;
    bool isTemp = false;

    if (!customFile.empty()) {
        testFile = toWide(customFile);
        std::cout << "[Benchmark] Testing against existing file: " << customFile << "\n";
    } else {
        testFile = L"munin_cli_benchmark.tmp";
        isTemp = true;
        uint64_t lines = 250000;
        std::cout << "[1] Generating synthetic log with " << lines << " lines...\n";
        auto t0 = std::chrono::high_resolution_clock::now();
        generateSyntheticLog(testFile, lines);
        auto t1 = std::chrono::high_resolution_clock::now();
        std::cout << "    Generated in " << std::chrono::duration<double>(t1 - t0).count() << " s\n";
    }

    MemoryMappedFile mmap;
    if (!mmap.open(testFile)) {
        std::cerr << "Failed to map file: " << mmap.lastError() << "\n";
        return 1;
    }

    double sizeMB = static_cast<double>(mmap.size()) / (1024.0 * 1024.0);
    std::cout << "[2] MemoryMappedFile size: " << std::fixed << std::setprecision(2) << sizeMB << " MB\n";

    ChunkIndex index;
    std::cout << "[3] Testing ChunkIndex with SIMD line scanner...\n";
    auto tIdx0 = std::chrono::high_resolution_clock::now();
    index.startIndexing(mmap.data(), mmap.size());
    while (index.isIndexing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    auto tIdx1 = std::chrono::high_resolution_clock::now();
    double idxSec = std::chrono::duration<double>(tIdx1 - tIdx0).count();

    std::cout << "    Total lines indexed: " << index.totalLines() << "\n";
    std::cout << "    Chunks created: " << index.chunkCount() << "\n";
    std::cout << "    Indexing time: " << idxSec << " s\n";
    if (idxSec > 0.0001) {
        std::cout << "    Indexing speed: " << (sizeMB / idxSec) << " MB/s\n";
    }

    std::cout << "[4] Testing Multi-Core ParallelSearch for term 'ERROR'...\n";
    SearchOptions opt;
    opt.query = "ERROR";
    ParallelSearch searcher;
    bool done = false;
    size_t found = 0;
    double searchTime = 0.0;

    searcher.startSearch(
        mmap.data(),
        mmap.size(),
        index,
        opt,
        nullptr,
        [&](const std::vector<SearchMatch>& matches, double elapsed) {
            found = matches.size();
            searchTime = elapsed;
            done = true;
        }
    );

    while (!done && searcher.isSearching()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    std::cout << "    Matches found: " << found << "\n";
    std::cout << "    Search elapsed: " << searchTime << " s\n";
    if (searchTime > 0.0001) {
        std::cout << "    Search throughput: " << (sizeMB / searchTime) << " MB/s\n";
    }

    mmap.close();
    if (isTemp) {
        DeleteFileW(testFile.c_str());
        std::cout << "    Cleaned up temporary benchmark file.\n";
    }

    std::cout << "\n[✓] Benchmark completed successfully!\n";
    return 0;
}

int main(int argc, char* argv[]) {
#if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc < 2) {
        printHelp();
        return 0;
    }

    std::string command = argv[1];

    if (command == "--help" || command == "-h" || command == "/?") {
        printHelp();
        return 0;
    }

    if (command == "--version" || command == "-v") {
        std::cout << "munin-app-cli version " << APP_VERSION << " (Windows x64)\n";
        return 0;
    }

    if (command == "bench" || command == "benchmark") {
        std::string customFile = (argc > 2) ? argv[2] : "";
        return cmdBench(customFile);
    }

    if (command == "info") {
        if (argc < 3) {
            std::cerr << "Error: 'info' command requires a file path.\n";
            std::cerr << "Usage: munin-app-cli.exe info <file>\n";
            return 1;
        }
        return cmdInfo(argv[2]);
    }

    if (command == "tail") {
        if (argc < 3) {
            std::cerr << "Error: 'tail' command requires a file path.\n";
            std::cerr << "Usage: munin-app-cli.exe tail <file>\n";
            return 1;
        }
        return cmdTail(argv[2]);
    }

    if (command == "search") {
        if (argc < 4) {
            std::cerr << "Error: 'search' command requires <file> and <term>.\n";
            std::cerr << "Usage: munin-app-cli.exe search <file> <term> [options]\n";
            return 1;
        }
        std::string filePath = argv[2];
        std::string query = argv[3];

        bool caseSensitive = true;
        bool isRegex = false;
        bool invert = false;
        size_t maxMatches = 1000;
        std::string format = "text";
        bool noBanner = false;

        for (int i = 4; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--ignore-case" || arg == "-i") {
                caseSensitive = false;
            } else if (arg == "--case-sensitive" || arg == "-c") {
                caseSensitive = true;
            } else if (arg == "--regex" || arg == "-r") {
                isRegex = true;
            } else if (arg == "--invert" || arg == "-v") {
                invert = true;
            } else if (arg == "--no-banner") {
                noBanner = true;
            } else if (arg == "--format" && i + 1 < argc) {
                format = argv[++i];
            } else if (arg == "--max" && i + 1 < argc) {
                maxMatches = static_cast<size_t>(std::strtoul(argv[++i], nullptr, 10));
            }
        }

        return cmdSearch(filePath, query, caseSensitive, isRegex, invert, maxMatches, format, noBanner);
    }

    std::cerr << "Unknown command: '" << command << "'\n";
    std::cerr << "Run 'munin-app-cli.exe --help' for usage.\n";
    return 1;
}
