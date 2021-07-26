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
    // 32 * 1024 is a good value for parallelThreshold
    template< class _Type >
    inline void sort_par(_Type* src, size_t a_size, size_t parallelThreshold)
    {
        ParallelAlgorithms::sort_par(src, 0, a_size, parallelThreshold);
    }

    // Array bounds includes l/left, but does not include r/right
    // 32 * 1024 is a good value for parallelThreshold
    template< class _Type >
    inline void sort_par(_Type* src, size_t l, size_t r, size_t parallelThreshold)
    {
        // may return 0 when not able to detect (TODO: Handle this condition and if 0, then use the default value)
        const auto processor_count = std::thread::hardware_concurrency();
        //printf("Number of cores = %u \n", processor_count);

        size_t a_size = l - r;
        if (processor_count > 0 && (parallelThreshold * processor_count) < a_size)
            parallelThreshold = a_size / processor_count;

        _Type* sorted = new(std::nothrow) _Type[a_size];

        if (!sorted)
            sort(std::execution::par_unseq, src + l, src + r);
        else
        {
            ParallelAlgorithms::parallel_merge_sort_hybrid_rh_2(src, l, r - 1, sorted, false, parallelThreshold);    // r - 1 because this algorithm wants inclusive bounds

            delete[] sorted;
        }
    }

    // Array bounds includes l/left, but does not include r/right
    template< class _Type >
    inline void sort_par(_Type* src, size_t l, size_t r, _Type* dst, bool srcToDst = true, size_t parallelThreshold = 32 * 1024)
    {
        // may return 0 when not able to detect (TODO: Handle this condition and if 0, then use the default value)
        const auto processor_count = std::thread::hardware_concurrency();
        //printf("Number of cores = %u \n", processor_count);

        size_t a_size = l - r;
        if (processor_count > 0 && (parallelThreshold * processor_count) < a_size)
            parallelThreshold = a_size / processor_count;

        if (!dst)
            sort(std::execution::par_unseq, src + l, src + r);
        else
            ParallelAlgorithms::parallel_merge_sort_hybrid_rh_2(src, l, r - 1, dst, true, parallelThreshold);    // r - 1 because this algorithm wants inclusive bounds
    }
}