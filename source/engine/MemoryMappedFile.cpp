#include "MemoryMappedFile.hpp"

#include <algorithm>
#include <vector>

namespace Munin::Engine {

MemoryMappedFile::MemoryMappedFile() = default;

MemoryMappedFile::~MemoryMappedFile() {
    cleanup();
}

MemoryMappedFile::MemoryMappedFile(MemoryMappedFile&& other) noexcept
    : m_fileHandle(other.m_fileHandle)
    , m_mappingHandle(other.m_mappingHandle)
    , m_mappedData(other.m_mappedData)
    , m_fileSize(other.m_fileSize)
    , m_filePath(std::move(other.m_filePath))
    , m_lastError(std::move(other.m_lastError)) {
    other.m_fileHandle = INVALID_HANDLE_VALUE;
    other.m_mappingHandle = nullptr;
    other.m_mappedData = nullptr;
    other.m_fileSize = 0;
}

MemoryMappedFile& MemoryMappedFile::operator=(MemoryMappedFile&& other) noexcept {
    if (this != &other) {
        cleanup();
        m_fileHandle = other.m_fileHandle;
        m_mappingHandle = other.m_mappingHandle;
        m_mappedData = other.m_mappedData;
        m_fileSize = other.m_fileSize;
        m_filePath = std::move(other.m_filePath);
        m_lastError = std::move(other.m_lastError);

        other.m_fileHandle = INVALID_HANDLE_VALUE;
        other.m_mappingHandle = nullptr;
        other.m_mappedData = nullptr;
        other.m_fileSize = 0;
    }
    return *this;
}

void MemoryMappedFile::cleanup() {
    if (m_mappedData) {
        ::UnmapViewOfFile(m_mappedData);
        m_mappedData = nullptr;
    }
    if (m_mappingHandle) {
        ::CloseHandle(m_mappingHandle);
        m_mappingHandle = nullptr;
    }
    if (m_fileHandle != INVALID_HANDLE_VALUE) {
        ::CloseHandle(m_fileHandle);
        m_fileHandle = INVALID_HANDLE_VALUE;
    }
    m_fileSize = 0;
}

void MemoryMappedFile::close() {
    cleanup();
    m_filePath.clear();
    m_lastError.clear();
}

std::string MemoryMappedFile::formatWin32Error(DWORD errorCode) {
    if (errorCode == 0) return {};
    LPSTR messageBuffer = nullptr;
    DWORD size = ::FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr);

    std::string message(messageBuffer, size);
    ::LocalFree(messageBuffer);
    while (!message.empty() && (message.back() == '\r' || message.back() == '\n' || message.back() == ' ')) {
        message.pop_back();
    }
    return message;
}

bool MemoryMappedFile::open(const std::wstring& path) {
    close();
    m_filePath = path;

    m_fileHandle = ::CreateFileW(
        path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        nullptr
    );

    if (m_fileHandle == INVALID_HANDLE_VALUE) {
        m_lastError = "CreateFileW failed: " + formatWin32Error(::GetLastError());
        return false;
    }

    LARGE_INTEGER fileSizeLi;
    if (!::GetFileSizeEx(m_fileHandle, &fileSizeLi)) {
        m_lastError = "GetFileSizeEx failed: " + formatWin32Error(::GetLastError());
        cleanup();
        return false;
    }

    m_fileSize = static_cast<uint64_t>(fileSizeLi.QuadPart);

    if (m_fileSize == 0) {
        // Empty file is valid, data is nullptr
        m_mappedData = nullptr;
        return true;
    }

    m_mappingHandle = ::CreateFileMappingW(
        m_fileHandle,
        nullptr,
        PAGE_READONLY,
        0,
        0,
        nullptr
    );

    if (!m_mappingHandle) {
        m_lastError = "CreateFileMappingW failed: " + formatWin32Error(::GetLastError());
        cleanup();
        return false;
    }

    LPVOID view = ::MapViewOfFile(
        m_mappingHandle,
        FILE_MAP_READ,
        0,
        0,
        0
    );

    if (!view) {
        m_lastError = "MapViewOfFile failed: " + formatWin32Error(::GetLastError());
        cleanup();
        return false;
    }

    m_mappedData = static_cast<const char*>(view);
    return true;
}

bool MemoryMappedFile::remap(uint64_t explicitNewSize) {
    if (m_fileHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    uint64_t newSize = explicitNewSize;
    if (newSize == 0) {
        LARGE_INTEGER li;
        if (!::GetFileSizeEx(m_fileHandle, &li)) {
            m_lastError = "GetFileSizeEx during remap failed: " + formatWin32Error(::GetLastError());
            return false;
        }
        newSize = static_cast<uint64_t>(li.QuadPart);
    }

    if (newSize == m_fileSize) {
        return true; // No change in size
    }

    // Unmap existing view and close mapping handle
    if (m_mappedData) {
        ::UnmapViewOfFile(m_mappedData);
        m_mappedData = nullptr;
    }
    if (m_mappingHandle) {
        ::CloseHandle(m_mappingHandle);
        m_mappingHandle = nullptr;
    }

    m_fileSize = newSize;

    if (m_fileSize == 0) {
        return true;
    }

    m_mappingHandle = ::CreateFileMappingW(
        m_fileHandle,
        nullptr,
        PAGE_READONLY,
        0,
        0,
        nullptr
    );

    if (!m_mappingHandle) {
        m_lastError = "CreateFileMappingW during remap failed: " + formatWin32Error(::GetLastError());
        return false;
    }

    LPVOID view = ::MapViewOfFile(
        m_mappingHandle,
        FILE_MAP_READ,
        0,
        0,
        0
    );

    if (!view) {
        m_lastError = "MapViewOfFile during remap failed: " + formatWin32Error(::GetLastError());
        return false;
    }

    m_mappedData = static_cast<const char*>(view);
    return true;
}

std::string_view MemoryMappedFile::slice(uint64_t offset, uint64_t length) const noexcept {
    if (!m_mappedData || offset >= m_fileSize) {
        return {};
    }
    uint64_t available = m_fileSize - offset;
    uint64_t actualLength = std::min(length, available);
    return std::string_view(m_mappedData + offset, static_cast<size_t>(actualLength));
}

} // namespace Munin::Engine
