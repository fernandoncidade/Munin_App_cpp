#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace Munin::Engine {

class SimdUtils {
public:
    // Fast newline counter using 64-bit SWAR / SSE2
    static size_t countNewlines(const char* data, size_t size) noexcept;

    // Finds the offset of the N-th newline (0-indexed: 0 means the 1st newline)
    // Returns true if found, sets foundOffset to the position of that '\n'.
    static bool findNthNewline(const char* data, size_t size, size_t targetIndex, size_t& foundOffset) noexcept;

    // Finds the next newline character starting from startOffset up to size.
    // If not found, returns size.
    static size_t findNextNewline(const char* data, size_t size, size_t startOffset) noexcept;
};

} // namespace Munin::Engine
