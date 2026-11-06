#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <stop_token>
#include <string>
#include <thread>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace Munin::Engine {

class LiveTailWatcher {
public:
    using FileGrownCallback = std::function<void(uint64_t oldSize, uint64_t newSize)>;

    LiveTailWatcher();
    ~LiveTailWatcher();

    // Starts watching the given file path
    bool start(const std::wstring& filePath, uint64_t initialSize, FileGrownCallback onFileGrown);

    // Stops watching
    void stop();

    // Temporarily pause/resume triggering notifications
    void setPaused(bool paused) noexcept { m_isPaused.store(paused, std::memory_order_relaxed); }
    [[nodiscard]] bool isPaused() const noexcept { return m_isPaused.load(std::memory_order_relaxed); }
    [[nodiscard]] bool isRunning() const noexcept { return m_isRunning.load(std::memory_order_relaxed); }

    // Update current known size (after manual remap)
    void updateCurrentSize(uint64_t newSize) noexcept { m_currentSize.store(newSize, std::memory_order_relaxed); }

private:
    void watchWorker(std::stop_token stopToken, std::wstring filePath, std::wstring directoryPath);
    static uint64_t queryFileSize(const std::wstring& path);

    std::wstring m_filePath;
    std::atomic<uint64_t> m_currentSize{0};
    std::atomic<bool> m_isRunning{false};
    std::atomic<bool> m_isPaused{false};

    std::jthread m_watchThread;
    FileGrownCallback m_onFileGrown;
};

} // namespace Munin::Engine
