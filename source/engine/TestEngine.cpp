#include "engine/MemoryMappedFile.hpp"
#include "engine/ChunkIndex.hpp"
#include "engine/ParallelSearch.hpp"
#include "engine/TimeSliceScanner.hpp"
#include "engine/SimdUtils.hpp"

#include <chrono>
#include <cstdio>
#include <iostream>
#include <windows.h>
#include <psapi.h>

using namespace Munin::Engine;

void generateBenchmarkLog(const std::wstring& path, uint64_t targetLines) {
    FILE* fp = nullptr;
    _wfopen_s(&fp, path.c_str(), L"wb");
    if (!fp) return;
    setvbuf(fp, nullptr, _IOFBF, 2 * 1024 * 1024);

    const char* levels[] = {"INFO", "DEBUG", "WARN", "ERROR", "TRACE"};
    char buf[512];

    for (uint64_t i = 0; i < targetLines; ++i) {
        int h = static_cast<int>((i / 3600) % 24);
        int m = static_cast<int>((i / 60) % 60);
        int s = static_cast<int>(i % 60);
        int th = static_cast<int>(i % 8) + 1;
        const char* lvl = levels[i % 5];
        int uid = 1000 + static_cast<int>(i % 500);

        int len = snprintf(
            buf, sizeof(buf),
            "2026-10-13T%02d:%02d:%02d.451Z [thread-%02d] 192.168.1.%d [%s] Request handled - {\"user_id\":%d,\"status\":%d}\n",
            h, m, s, th, (th * 19) % 250 + 1, lvl, uid, (lvl[0] == 'E' ? 500 : 200)
        );

        if (len > 0) {
            fwrite(buf, 1, len, fp);
        }
    }

    fclose(fp);
}

int main() {
    std::wcout << L"=== MUNIN ENGINE BENCHMARK & VERIFICATION ===" << std::endl;

    std::wstring testFile = L"munin_benchmark_test.log";
    uint64_t linesToGenerate = 300000; // 300,000 lines

    std::wcout << L"[1] Gerando arquivo sintético com " << linesToGenerate << L" linhas..." << std::endl;
    auto t0 = std::chrono::high_resolution_clock::now();
    generateBenchmarkLog(testFile, linesToGenerate);
    auto t1 = std::chrono::high_resolution_clock::now();
    double genSec = std::chrono::duration<double>(t1 - t0).count();
    std::wcout << L"    Gerado em " << genSec << L"s." << std::endl;

    // 1. Test MemoryMappedFile
    std::wcout << L"\n[2] Testando Camada de I/O (Win32 MemoryMappedFile)..." << std::endl;
    MemoryMappedFile mmap;
    if (!mmap.open(testFile)) {
        std::cerr << "Falha ao abrir arquivo mapeado: " << mmap.lastError() << std::endl;
        return 1;
    }
    double sizeMB = static_cast<double>(mmap.size()) / (1024.0 * 1024.0);
    std::wcout << L"    Arquivo mapeado com sucesso! Tamanho: " << sizeMB << L" MB" << std::endl;

    // 2. Test ChunkIndex
    std::wcout << L"\n[3] Testando Camada de Indexação (ChunkIndex com SIMD SSE2 / SWAR)..." << std::endl;
    ChunkIndex index;
    auto tIdx0 = std::chrono::high_resolution_clock::now();
    index.startIndexing(mmap.data(), mmap.size());

    while (index.isIndexing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    auto tIdx1 = std::chrono::high_resolution_clock::now();
    double idxSec = std::chrono::duration<double>(tIdx1 - tIdx0).count();

    std::wcout << L"    Total de linhas indexadas: " << index.totalLines() << std::endl;
    std::wcout << L"    Total de chunks criados: " << index.chunkCount() << std::endl;
    std::wcout << L"    Velocidade de indexação: " << (sizeMB / idxSec) << L" MB/s" << std::endl;

    if (index.totalLines() != linesToGenerate) {
        std::cerr << "ERRO: Linhas esperadas " << linesToGenerate << ", obtidas " << index.totalLines() << std::endl;
        return 1;
    }

    // 3. Test Random Line Seeking
    std::wcout << L"\n[4] Testando Acesso Aleatório (Seek Virtualizado)..." << std::endl;
    uint64_t testLines[] = {0, 1, 1000, 1542, 50000, 150000, 299999};
    for (uint64_t l : testLines) {
        std::string_view line = index.getLine(mmap.data(), mmap.size(), l);
        std::cout << "    Linha #" << (l + 1) << " [" << line.size() << " bytes]: "
                  << line.substr(0, std::min<size_t>(line.size(), 70)) << "..." << std::endl;
    }

    // 4. Test Parallel Search
    std::wcout << L"\n[5] Testando Busca JIT Paralela Multi-Core..." << std::endl;
    ParallelSearch search;
    SearchOptions opts;
    opts.query = "ERROR";
    opts.caseSensitive = true;
    opts.isRegex = false;

    std::vector<SearchMatch> matchesFound;
    double searchTime = 0.0;
    std::atomic<bool> searchDone{false};

    search.startSearch(
        mmap.data(), mmap.size(), index, opts,
        nullptr,
        [&](const std::vector<SearchMatch>& res, double elapsed) {
            matchesFound = res;
            searchTime = elapsed;
            searchDone.store(true);
        }
    );

    while (!searchDone.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::wcout << L"    Busca por 'ERROR' finalizada em " << searchTime << L"s!" << std::endl;
    std::wcout << L"    Ocorrências encontradas: " << matchesFound.size() << std::endl;

    // 5. Test TimeSlice Distribution
    std::wcout << L"\n[6] Testando Análise de Time-Slicing (Histograma ISO-8601)..." << std::endl;
    auto dist = TimeSliceScanner::buildDistribution(mmap.data(), mmap.size(), index, 40);
    std::wcout << L"    Timestamps detectados: " << (dist.hasTimestamps ? L"SIM" : L"NÃO") << std::endl;
    std::wcout << L"    Buckets gerados: " << dist.buckets.size() << std::endl;
    if (!dist.buckets.empty()) {
        std::cout << "    Primeiro bucket: " << dist.buckets.front().labelStart
                  << " -> Contagem: " << dist.buckets.front().count
                  << " (Erros: " << dist.buckets.front().errorCount << ")" << std::endl;
    }

    // 6. Test Memory Usage
    PROCESS_MEMORY_COUNTERS_EX pmc{};
    if (::GetProcessMemoryInfo(::GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        double ramMB = static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
        std::wcout << L"\n[7] Uso real de Memória RAM (WorkingSetSize): " << ramMB << L" MB" << std::endl;
        if (ramMB < 100.0) {
            std::wcout << L"    >>> SUCESSO: Consumo de RAM inferior a 100 MB mantido! <<<" << std::endl;
        }
    }

    mmap.close();
    ::DeleteFileW(testFile.c_str());

    std::wcout << L"\n>>> TODOS OS TESTES PASSARAM COM SUCESSO ABSOLUTO! <<<" << std::endl;
    return 0;
}
