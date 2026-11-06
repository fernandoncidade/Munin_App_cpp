#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace Munin::Engine {

class MemoryMappedFile {
public:
    MemoryMappedFile();
    ~MemoryMappedFile();

    // Move-only semantics for safe Win32 handle ownership
    MemoryMappedFile(const MemoryMappedFile&) = delete;
    MemoryMappedFile& operator=(const MemoryMappedFile&) = delete;
    MemoryMappedFile(MemoryMappedFile&& other) noexcept;
    MemoryMappedFile& operator=(MemoryMappedFile&& other) noexcept;

    // Opens and memory-maps the file in read-only mode with shared read/write access
    bool open(const std::wstring& path);

    // Closes any open handles and unmaps the view
    void close();

    // Expands or remaps the file mapping when the file has grown (for Live Tail)
    bool remap(uint64_t explicitNewSize = 0);

    [[nodiscard]] bool isOpen() const noexcept { return m_mappedData != nullptr || (m_fileHandle != INVALID_HANDLE_VALUE && m_fileSize == 0); }
    [[nodiscard]] const char* data() const noexcept { return m_mappedData; }
    [[nodiscard]] uint64_t size() const noexcept { return m_fileSize; }
    [[nodiscard]] const std::wstring& filePath() const noexcept { return m_filePath; }
    [[nodiscard]] const std::string& lastError() const noexcept { return m_lastError; }

    // Obtains a safe slice of data. Clamps bounds if exceeding file size.
    [[nodiscard]] std::string_view slice(uint64_t offset, uint64_t length) const noexcept;

private:
    void cleanup();
    static std::string formatWin32Error(DWORD errorCode);

    HANDLE m_fileHandle{INVALID_HANDLE_VALUE};
    HANDLE m_mappingHandle{nullptr};
    const char* m_mappedData{nullptr};
    uint64_t m_fileSize{0};
    std::wstring m_filePath;
    std::string m_lastError;
};

} // namespace Munin::Engine
