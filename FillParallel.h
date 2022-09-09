// TODO: Possible improvement is to provide an option to go around the CPU cache using SSE instructions for writes that can go around the cache, to not evict items out of the cache.
// Parallel Fill implementations

#ifndef _ParallelFill_h
#define _ParallelFill_h

//#include <cstddef>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <thread>
#include <execution>
#include <ppl.h>
#include <algorithm>
#else
#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <ratio>
#include <vector>
#include <thread>
#include <execution>
#endif

namespace ParallelAlgorithms
{
    // Inclusive-left and exclusive-right boundaries
    template< class _Type >
    inline void parallel_fill(_Type* src, _Type value, size_t l, size_t r, size_t parallel_threshold = 16 * 1024)
    {
        if (r <= l)
            return;
        if ((r - l) < parallel_threshold)
        {
            std::fill(src + l, src + r, value);     // many times faster than for loop
            //for (size_t i = l; i < r; i++)
            //    src[i] = value;
            return;
        }
        size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;     // average without overflow
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        Concurrency::parallel_invoke(
#else
        tbb::parallel_invoke(
#endif
            [&] { parallel_fill(src, value, l, m, parallel_threshold); },
            [&] { parallel_fill(src, value, m, r, parallel_threshold); }
        );
    }
    // Inclusive-left and exclusive-right boundaries
    inline void parallel_fill(unsigned char* src, unsigned char value, size_t l, size_t r, size_t parallel_threshold = 16 * 1024)
    {
        if (r <= l)
            return;
        if ((r - l) < parallel_threshold)
        {
            //memset(src + l, (int)value, r - l);   // many times faster than the for loop below
            std::fill(src + l, src + r, value);     // same performance as memset
            //for (size_t i = l; i < r; i++)
            //    src[i] = value;
            return;
        }
        size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;     // average without overflow
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        Concurrency::parallel_invoke(
#else
        tbb::parallel_invoke(
#endif
            [&] { parallel_fill(src, value, l, m, parallel_threshold); },
            [&] { parallel_fill(src, value, m, r, parallel_threshold); }
        );
    }
}
#endif