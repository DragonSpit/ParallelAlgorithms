// TODO: Benchmark how long memory allocation takes
// TODO: Benchmark how much better algorithm does where dst/working buffer is provided, versus one that is provided and paged in
#pragma once

#include "Configuration.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <ratio>
#include <vector>
#include <thread>
#include <execution>

#include "ParallelMergeSort.h"

namespace ParallelAlgorithms
{
    // Sort the entire array of any data type with comparable elements
    // Adaptive algorithm: if enough memory to allocate a temporary working buffer, then faster not-in-place parallel merge sort is used.
    //                     if not enough memory, then the standard C++ in-place parallel sort is used, which is slower.
    template< class _Type >
    inline void sort_par(_Type* src, size_t src_size)
    {
        ParallelAlgorithms::sort_par(src, 0, src_size);
    }

    template< class _Type >
    inline void sort_par(std::vector<_Type>& src)
    {
        ParallelAlgorithms::sort_par(src, 0, src.size());
    }

    // Array bounds includes l/left, but does not include r/right
    template< class _Type >
    inline void sort_par(_Type* src, size_t l, size_t r)
    {
        size_t src_size = r;
        _Type* sorted = new(std::nothrow) _Type[src_size];

        if (!sorted)
            sort(std::execution::par_unseq, src + l, src + r);
        else
        {
            ParallelAlgorithms::parallel_merge_sort_hybrid_rh_1(src, l, r - 1, sorted, false);    // r - 1 because this algorithm wants inclusive bounds

            delete[] sorted;
        }
    }

    // Array bounds includes l/left, but does not include r/right
    template< class _Type >
    inline void sort_par(std::vector<_Type>& src, size_t l, size_t r)
    {
        try
        {
            size_t src_size = r;
            std::vector<_Type> sorted(src_size);
            ParallelAlgorithms::parallel_merge_sort_hybrid_rh_1(src.data(), l, r - 1, sorted.data(), false);    // r - 1 because this algorithm wants inclusive bounds
        }
        catch (std::bad_alloc& ba)
        {
            sort(std::execution::par_unseq, src.begin() + l, src.begin() + r);
        }
    }

    // dst buffer must be the same or larger in size than the src
    // Two use cases:
    //   -     in-place interface, where the dst buffer is a temporary work buffer
    //   - not-in-place interface, where the dst buffer is the destination memory buffer
    template< class _Type >
    inline void sort_par(_Type* src, size_t src_size, _Type* dst, size_t dst_size, bool srcToDst = false)
    {
        ParallelAlgorithms::sort_par(src, 0, src_size, dst, dst_size, srcToDst);
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

        ParallelAlgorithms::parallel_merge_sort_hybrid_rh_2(src, l, r - 1, dst, srcToDst);    // r - 1 because this algorithm wants inclusive bounds
    }
}