#include "LiveTailWatcher.hpp"

#include <chrono>
#include <filesystem>

namespace Munin::Engine {

LiveTailWatcher::LiveTailWatcher() = default;

LiveTailWatcher::~LiveTailWatcher() {
    stop();
}

void LiveTailWatcher::stop() {
    if (m_watchThread.joinable()) {
        m_watchThread.request_stop();
        m_watchThread.join();
    }
    m_isRunning.store(false, std::memory_order_relaxed);
}

uint64_t LiveTailWatcher::queryFileSize(const std::wstring& path) {
    WIN32_FILE_ATTRIBUTE_DATA fileData{};
    if (::GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &fileData)) {
        ULARGE_INTEGER uli;
        uli.LowPart = fileData.nFileSizeLow;
        uli.HighPart = fileData.nFileSizeHigh;
        return uli.QuadPart;
    }
    return 0;
}

bool LiveTailWatcher::start(const std::wstring& filePath, uint64_t initialSize, FileGrownCallback onFileGrown) {
    stop();

    if (filePath.empty()) return false;

    m_filePath = filePath;
    m_currentSize.store(initialSize, std::memory_order_relaxed);
    m_onFileGrown = std::move(onFileGrown);
    m_isPaused.store(false, std::memory_order_relaxed);

    std::filesystem::path p(filePath);
    std::wstring dirPath = p.parent_path().wstring();
    if (dirPath.empty()) {
        dirPath = L".";
    }

    m_isRunning.store(true, std::memory_order_relaxed);

    m_watchThread = std::jthread([this, filePath, dirPath](std::stop_token stopToken) {
        watchWorker(stopToken, filePath, dirPath);
    });

    return true;
}

void LiveTailWatcher::watchWorker(std::stop_token stopToken, std::wstring filePath, std::wstring dirPath) {
    HANDLE changeHandle = ::FindFirstChangeNotificationW(
        dirPath.c_str(),
        FALSE,
        FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE
    );

    while (!stopToken.stop_requested()) {
        DWORD waitResult = WAIT_TIMEOUT;
        if (changeHandle != INVALID_HANDLE_VALUE) {
            waitResult = ::WaitForSingleObject(changeHandle, 150); // Poll every 150ms
        } else {
            // Fallback sleep if notification handle failed
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        if (stopToken.stop_requested()) break;

        uint64_t newSize = queryFileSize(filePath);
        uint64_t oldSize = m_currentSize.load(std::memory_order_relaxed);

        if (newSize > oldSize) {
            m_currentSize.store(newSize, std::memory_order_relaxed);
            if (!m_isPaused.load(std::memory_order_relaxed) && m_onFileGrown) {
                m_onFileGrown(oldSize, newSize);
            }
        }

        if (changeHandle != INVALID_HANDLE_VALUE && waitResult == WAIT_OBJECT_0) {
            ::FindNextChangeNotification(changeHandle);
        }
    }

    if (changeHandle != INVALID_HANDLE_VALUE) {
        ::FindCloseChangeNotification(changeHandle);
    }

    m_isRunning.store(false, std::memory_order_relaxed);
}

} // namespace Munin::Engine
