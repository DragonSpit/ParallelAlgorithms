// TODO: Benchmark how long memory allocation takes
// TODO: Benchmark how much better algorithm does where dst/working buffer is provided, versus one that is provided and paged in
#pragma once

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

#include "ParallelMergeSort.h"

namespace ParallelAlgorithms
{
    // Sort the entire array of any data type with comparable elements
    template< class _Type >
    inline void sort_par(_Type* src, size_t src_size)
    {
        ParallelAlgorithms::sort_par(src, 0, src_size);
    }

    // Array bounds includes l/left, but does not include r/right
    template< class _Type >
    inline void sort_par(_Type* src, size_t l, size_t r)
    {
        _Type* sorted = new(std::nothrow) _Type[a_size];

        if (!sorted)
            sort(std::execution::par_unseq, src + l, src + r);
        else
        {
            ParallelAlgorithms::parallel_merge_sort_hybrid_rh_1(src, l, r - 1, sorted, false);    // r - 1 because this algorithm wants inclusive bounds

            delete[] sorted;
        }
    }

    // dst buffer must be the same or larger in size than the src
    // Two use cases:
    //   -     in-place interface, where the dst buffer is a temporary work buffer
    //   - not-in-place interface, where the dst buffer is the destination memory buffer
    template< class _Type >
    inline void sort_par(_Type* src, size_t src_size, _Type* dst, size_t dst_size, bool srcToDst = false)
    {
        if (!dst)
            throw std::invalid_argument("dst is null, which is not supported");
        if (dst_size < src_size)
            throw std::invalid_argument("dst_size must be larger or equal to src_size");

        ParallelAlgorithms::parallel_merge_sort_hybrid_rh_1(src, 0, src_size - 1, dst, srcToDst);    // size - 1 because this algorithm wants inclusive bounds
    }

    // Array bounds includes l/left, but does not include r/right
    // dst buffer must be large enough to provide elements dst[0 to r-1], as the result is placed in dst[l to r-1]
    // Two use cases:
    //   -     in-place interface, where the dst buffer is a temporary work buffer
    //   - not-in-place interface, where the dst buffer is the destination memory buffer
    template< class _Type >
    inline void sort_par(_Type* src, size_t l, size_t r, _Type* dst, size_t dst_size, bool srcToDst = false)
    {
        if (!dst)
            throw std::invalid_argument("dst is null, which is not supported");
        size_t src_size = r;
        if (dst_size < src_size)
            throw std::invalid_argument("dst_size must be larger or equal to r, to be able to return dst[l to r-1]");

        ParallelAlgorithms::parallel_merge_sort_hybrid_rh_1(src, l, r - 1, dst, srcToDst);    // r - 1 because this algorithm wants inclusive bounds
    }
}