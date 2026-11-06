#include "SimdUtils.hpp"

#include <bit>
#include <cstring>

#if defined(_M_X64) || defined(__x86_64__)
#include <emmintrin.h>
#endif

namespace Munin::Engine {

size_t SimdUtils::countNewlines(const char* data, size_t size) noexcept {
    if (!data || size == 0) return 0;

    size_t count = 0;
    size_t i = 0;

#if defined(_M_X64) || defined(__x86_64__)
    const __m128i target = _mm_set1_epi8('\n');
    const size_t simdEnd = size >= 16 ? size - 15 : 0;

    for (; i < simdEnd; i += 16) {
        __m128i block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + i));
        __m128i cmp = _mm_cmpeq_epi8(block, target);
        int mask = _mm_movemask_epi8(cmp);
        count += static_cast<size_t>(std::popcount(static_cast<unsigned int>(mask)));
    }
#endif

    // Process remaining bytes
    for (; i < size; ++i) {
        if (data[i] == '\n') {
            ++count;
        }
    }

    return count;
}

bool SimdUtils::findNthNewline(const char* data, size_t size, size_t targetIndex, size_t& foundOffset) noexcept {
    if (!data || size == 0) return false;

    size_t count = 0;
    size_t i = 0;

#if defined(_M_X64) || defined(__x86_64__)
    const __m128i target = _mm_set1_epi8('\n');
    const size_t simdEnd = size >= 16 ? size - 15 : 0;

    for (; i < simdEnd; i += 16) {
        __m128i block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + i));
        __m128i cmp = _mm_cmpeq_epi8(block, target);
        unsigned int mask = static_cast<unsigned int>(_mm_movemask_epi8(cmp));

        if (mask != 0) {
            unsigned int matchesInBlock = static_cast<unsigned int>(std::popcount(mask));
            if (count + matchesInBlock > targetIndex) {
                // The target newline is inside this 16-byte block
                while (mask != 0) {
                    int bitIndex = std::countr_zero(mask);
                    if (count == targetIndex) {
                        foundOffset = i + static_cast<size_t>(bitIndex);
                        return true;
                    }
                    ++count;
                    mask &= ~(1u << bitIndex);
                }
            }
            count += matchesInBlock;
        }
    }
#endif

    // Scalar fallback / remainder
    for (; i < size; ++i) {
        if (data[i] == '\n') {
            if (count == targetIndex) {
                foundOffset = i;
                return true;
            }
            ++count;
        }
    }

    return false;
}

size_t SimdUtils::findNextNewline(const char* data, size_t size, size_t startOffset) noexcept {
    if (!data || startOffset >= size) return size;

    size_t i = startOffset;

#if defined(_M_X64) || defined(__x86_64__)
    const __m128i target = _mm_set1_epi8('\n');
    const size_t simdEnd = size >= 16 ? size - 15 : 0;

    // Align to 16-byte loop if possible
    while (i < simdEnd && (reinterpret_cast<uintptr_t>(data + i) & 15) != 0) {
        if (data[i] == '\n') return i;
        ++i;
    }

    for (; i < simdEnd; i += 16) {
        __m128i block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + i));
        __m128i cmp = _mm_cmpeq_epi8(block, target);
        unsigned int mask = static_cast<unsigned int>(_mm_movemask_epi8(cmp));
        if (mask != 0) {
            int bitIndex = std::countr_zero(mask);
            return i + static_cast<size_t>(bitIndex);
        }
    }
#endif

    for (; i < size; ++i) {
        if (data[i] == '\n') return i;
    }

    return size;
}

} // namespace Munin::Engine
