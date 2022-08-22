// Parallel Fill implementations

#ifndef _ParallelFill_h
#define _ParallelFill_h

//#include <cstddef>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <thread>
#include <execution>
#include <ppl.h>
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
    inline void parallel_fill_r(unsigned char* src, unsigned char value, std::size_t l, std::size_t r, std::size_t parallel_threshold = 16 * 1024)
    {
        if (r < l)
            return;
        if ((r - l + 1) < parallel_threshold)
        {
            //memset(src + l, (int)value, r - l + 1);
            for (size_t i = l; i <= r; i++)
                src[i] = value;
            return;
        }
        size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;     // average without overflow
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        Concurrency::parallel_invoke(
#else
        tbb::parallel_invoke(
#endif
            // TODO: Figure out why when enabling the next two lines crashes!
            //[&] { parallel_fill_r(src, value, l,     m, parallel_threshold); },
            //[&] { parallel_fill_r(src, value, m + 1, r, parallel_threshold); }
            [&] { memset(src + l,     value, m - l + 1      ); },
            [&] { memset(src + m + 1, value, r - (m + 1) + 1); }
            //memset(src + l, (int)value, r - l + 1);
        );
    }

    inline void parallel_fill_r2(unsigned char* src, unsigned char value, std::size_t start_index, std::size_t length, std::size_t parallel_threshold = 16 * 1024)
    {
        //printf("pfr2: start_index = %zu   length = %zu\n", start_index, length);
        if (length < parallel_threshold)
        {
            memset(src + start_index, (int)value, length);
            //size_t end_index = start_index + length;
            //for (size_t i = start_index; i < end_index; i++)
            //    src[i] = value;
            return;
        }
        size_t m = length / 2;
        size_t length_of_2nd_half = length - m;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        Concurrency::parallel_invoke(
#else
        tbb::parallel_invoke(
#endif
            [=] { parallel_fill_r2(src, value, start_index,     m,                  parallel_threshold); },
            [=] { parallel_fill_r2(src, value, start_index + m, length_of_2nd_half, parallel_threshold); }
            //[&] { memset(src + l, value, m - l + 1); },
            //[&] { memset(src + m + 1, value, r - (m + 1) + 1); }
            //memset(src + l, (int)value, r - l + 1);
        );
    }

    template< class _Type >
    inline void parallel_fill(_Type* src, _Type value, std::size_t l, std::size_t r, std::size_t parallel_threshold = 16 * 1024)
    {
        if (r < l) return;
        parallel_fill_r(src, value, l, r, parallel_threshold);
    }
}
#endif