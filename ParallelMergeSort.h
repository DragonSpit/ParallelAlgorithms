// TODO: Place all of these algorithms in a parallel_algorithms namespace
// TODO: Provide the same interface to serial and parallel algorithms as standard C++ does, using the first argument as the execution policy
// TODO: Improve parallel in-place merge sort by using the same method as the not-in-place merge sort does, where it looks at how many processors are
//       available and adjusts the parallel threshold accordingly, as the current parallel threshold is set way too small.

// Parallel Merge Sort implementations

#ifndef _ParallelMergeSort_h
#define _ParallelMergeSort_h

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

#include "InsertionSort.h"
#include "BinarySearch.h"
#include "ParallelMerge.h"
#include "RadixSortLSD.h"
#include "RadixSortLsdParallel.h"

namespace ParallelAlgorithms
{
    // The simplest version of parallel merge sort that reverses direction of source and destination arrays on each level of recursion
    // to eliminate the use of an additional array.  The top-level of recursion starts in the source to destination direction, which is
    // what's needed and reverses direction at each level of recursion, handling the leaf nodes by using a copy when the direction is opposite.
    // Assumes l <= r on entrance, which is simple to check if really needed.
    // Think of srcDst as specifying the direction at this recursion level, and as recursion goes down what is passed in srcDst is control of
    // direction for that next level of recursion.
    // Will this work if the top-level srcToDst is set to false to begin with - i.e. we want the result to end up in the source buffer and use
    // the destination buffer as an auxilary buffer/storage.  It would be really cool if the algorithm just worked this way, and had these
    // two modes of usage.  I predict that it will just work that way, and then I may need to define two entrance point functions that make these
    // two behaviors more obvious and explicit and not even have srcToDst argument.
    // Indexes l and r must be int's to provide the ability to specify zero elements with l = 0 and r = -1.  Otherwise, specifying zero would be a little strange
    // and you'd have to do it as l = 1 and r = 0. !!! This may be the reason that STL does *src_start and *src_end, and then the wrapper function may not be needed!!!

    // Listing 1
    template< class _Type >
    inline void parallel_merge_sort_simplest_r(_Type* src, int l, int r, _Type* dst, bool srcToDst = true)	// srcToDst specifies direction for this level of recursion
    {
        if (r == l) {    // termination/base case of sorting a single element
            if (srcToDst)  dst[l] = src[l];    // copy the single element from src to dst
            return;
        }
        int m = (r + l) / 2;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        Concurrency::parallel_invoke(
#else
        tbb::parallel_invoke(
#endif
            [&] { parallel_merge_sort_simplest_r(src, l,     m, dst, !srcToDst); },		// reverse direction of srcToDst for the next level of recursion
            [&] { parallel_merge_sort_simplest_r(src, m + 1, r, dst, !srcToDst); }		// reverse direction of srcToDst for the next level of recursion
        );
        if (srcToDst)   merge_parallel_L5(src, l, m, m + 1, r, dst, l);
        else	        merge_parallel_L5(dst, l, m, m + 1, r, src, l);
    }

    template< class _Type >
    inline void parallel_merge_sort_simplest(_Type* src, int l, int r, _Type* dst, bool srcToDst = true)	// srcToDst specifies direction for this level of recursion
    {
        if (r < l) return;
        parallel_merge_sort_simplest_r(src, l, r, dst, srcToDst);
    }

    // Listing 2
    template< class _Type >
    inline void parallel_merge_sort(_Type* src, int l, int r, _Type* dst)
    {
        parallel_merge_sort_hybrid(src, l, r, dst, true);  // srcToDst = true
    }

    template< class _Type >
    inline void parallel_merge_sort_pseudo_inplace(_Type* srcDst, int l, int r, _Type* aux)
    {
        parallel_merge_sort_hybrid(srcDst, l, r, aux, false);  // srcToDst = false
    }

    // Listing 3
    template< class _Type >
    inline void parallel_merge_sort_hybrid_rh(_Type* src, int l, int r, _Type* dst, bool srcToDst = true)
    {
        if (r < l)  return;
        if (r == l) {    // termination/base case of sorting a single element
            if (srcToDst)  dst[l] = src[l];    // copy the single element from src to dst
            return;
        }
        if ((r - l) <= 48) {
            insertionSortSimilarToSTLnoSelfAssignment(src + l, r - l + 1);        // in both cases sort the src
            //stable_sort( src + l, src + r + 1 );  // STL stable_sort can be used instead, but is slightly slower than Insertion Sort
            if (srcToDst) for (int i = l; i <= r; i++)    dst[i] = src[i];    // copy from src to dst, when the result needs to be in dst
            return;
        }
        int m = (r + l) / 2;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        Concurrency::parallel_invoke(
#else
        tbb::parallel_invoke(
#endif
            [&] { parallel_merge_sort_hybrid_rh(src, l, m, dst, !srcToDst); },        // reverse direction of srcToDst for the next level of recursion
            [&] { parallel_merge_sort_hybrid_rh(src, m + 1, r, dst, !srcToDst); }     // reverse direction of srcToDst for the next level of recursion
        );
        if (srcToDst) merge_parallel_L5(src, l, m, m + 1, r, dst, l);
        else          merge_parallel_L5(dst, l, m, m + 1, r, src, l);
    }

    // Listing 4
    template< class _Type >
    inline void parallel_merge_sort_hybrid_rh_1(_Type* src, size_t l, size_t r, _Type* dst, bool srcToDst = true)
    {
        if (r < l)  return;
        if (r == l) {    // termination/base case of sorting a single element
            if (srcToDst)  dst[l] = src[l];    // copy the single element from src to dst
            return;
        }
        if ((r - l) <= 48 && !srcToDst) {     // 32 or 64 or larger seem to perform well
            insertionSortSimilarToSTLnoSelfAssignment(src + l, r - l + 1);    // want to do dstToSrc, can just do it in-place, just sort the src, no need to copy
            //stable_sort( src + l, src + r + 1 );  // STL stable_sort can be used instead, but is slightly slower than Insertion Sort
            return;
        }
        size_t m = ((r + l) / 2);
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        Concurrency::parallel_invoke(
#else
        tbb::parallel_invoke(
#endif
            [&] { parallel_merge_sort_hybrid_rh_1(src, l, m, dst, !srcToDst); },      // reverse direction of srcToDst for the next level of recursion
            [&] { parallel_merge_sort_hybrid_rh_1(src, m + 1, r, dst, !srcToDst); }       // reverse direction of srcToDst for the next level of recursion
        );
        if (srcToDst) merge_parallel_L5(src, l, m, m + 1, r, dst, l);
        else          merge_parallel_L5(dst, l, m, m + 1, r, src, l);
    }

    template< class _Type >
    inline void parallel_merge_sort_hybrid_rh_2(_Type* src, size_t l, size_t r, _Type* dst, bool srcToDst = true, size_t parallelThreshold = 32 * 1024)
    {
        if (r < l)  return;
        if (r == l) {   // termination/base case of sorting a single element
            if (srcToDst)  dst[l] = src[l];    // copy the single element from src to dst
            return;
        }
        if ((r - l) <= parallelThreshold && !srcToDst) {
            std::sort(src + l, src + r + 1);
            //if (srcToDst)
            //    for (int i = l; i <= r; i++)    dst[i] = src[i];
            return;
        }
        size_t m = (r + l) / 2;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        Concurrency::parallel_invoke(
#else
        tbb::parallel_invoke(
#endif
            [&] { parallel_merge_sort_hybrid_rh_2(src, l,     m, dst, !srcToDst); },      // reverse direction of srcToDst for the next level of recursion
            [&] { parallel_merge_sort_hybrid_rh_2(src, m + 1, r, dst, !srcToDst); }       // reverse direction of srcToDst for the next level of recursion
        );
        if (srcToDst) merge_parallel_L5(src, l, m, m + 1, r, dst, l);
        else          merge_parallel_L5(dst, l, m, m + 1, r, src, l);
    }

    template< class _Type >
    inline void parallel_merge_sort_hybrid(_Type* src, int l, int r, _Type* dst, bool srcToDst = true, int parallelThreshold = 24 * 1024)
    {
        // may return 0 when not able to detect
        const auto processor_count = std::thread::hardware_concurrency();
        //printf("Number of cores = %u \n", processor_count);

        if ((int)(parallelThreshold * processor_count) < (r - l + 1))
            parallelThreshold = (r - l + 1) / processor_count;

        parallel_merge_sort_hybrid_rh_2(src, l, r, dst, srcToDst, parallelThreshold);
    }

    inline void parallel_merge_sort_hybrid_radix_inner(unsigned long* src, size_t l, size_t r, unsigned long* dst, bool srcToDst = true, size_t parallelThreshold = 32 * 1024)
    {
        //printf("l = %zd   r = %zd   parallelThreshold = %zd\n", l, r, parallelThreshold);
        if (r < l)  return;
        if (r == l) {   // termination/base case of sorting a single element
            if (srcToDst)  dst[l] = src[l];    // copy the single element from src to dst
            return;
        }
        if ((r - l) <= parallelThreshold && !srcToDst) {
            RadixSortLSDPowerOf2Radix_unsigned_TwoPhase(src + l, dst + l, r - l + 1);
            //RadixSortLSDPowerOf2RadixParallel_unsigned_TwoPhase(src + l, dst + l, r - l + 1);
            //RadixSortLSDPowerOf2Radix_unsigned_TwoPhase_DeRandomize(src + l, dst + l, r - l + 1);
            //RadixSortLSDPowerOf2RadixParallel_unsigned_TwoPhase_DeRandomize(src + l, dst + l, r - l + 1);
            //if (srcToDst)
            //    for (int i = l; i <= r; i++)    dst[i] = src[i];
            return;
        }
        size_t m = (r + l) / 2;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        Concurrency::parallel_invoke(
#else
        tbb::parallel_invoke(
#endif
            [&] { parallel_merge_sort_hybrid_radix_inner(src, l,     m, dst, !srcToDst, parallelThreshold); },      // reverse direction of srcToDst for the next level of recursion
            [&] { parallel_merge_sort_hybrid_radix_inner(src, m + 1, r, dst, !srcToDst, parallelThreshold); }       // reverse direction of srcToDst for the next level of recursion
        );
        if (srcToDst) merge_parallel_L5(src, l, m, m + 1, r, dst, l);
        else          merge_parallel_L5(dst, l, m, m + 1, r, src, l);
    }

    inline void parallel_merge_sort_hybrid_radix(unsigned long* src, size_t l, size_t r, unsigned long* dst, bool srcToDst = true, size_t parallelThreshold = 24 * 1024)
    {
        // may return 0 when not able to detect
        const auto processor_count = std::thread::hardware_concurrency();
        //printf("Number of cores = %u   parallelThreshold = %d\n", processor_count, parallelThreshold);

        if ((parallelThreshold * processor_count) < (r - l + 1))
            parallelThreshold = (r - l + 1) / processor_count;

        parallel_merge_sort_hybrid_radix_inner(src, l, r, dst, srcToDst, parallelThreshold);
    }

    // Serial Merge Sort, using divide-and-conquer algorthm
    template< class _Type >
    inline void merge_sort_hybrid(_Type* src, size_t l, size_t r, _Type* dst, bool srcToDst = true)
    {
        if (r < l)  return;
        if (r == l) {    // termination/base case of sorting a single element
            if (srcToDst)  dst[l] = src[l];    // copy the single element from src to dst
            return;
        }
        if ((r - l) <= 48 && !srcToDst) {     // 32 or 64 or larger seem to perform well
            insertionSortSimilarToSTLnoSelfAssignment(src + l, r - l + 1);    // want to do dstToSrc, can just do it in-place, just sort the src, no need to copy
            //stable_sort( src + l, src + r + 1 );  // STL stable_sort can be used instead, but is slightly slower than Insertion Sort
            return;
        }
        size_t m = ((r + l) / 2);

        merge_sort_hybrid(src, l,     m, dst, !srcToDst);      // reverse direction of srcToDst for the next level of recursion
        merge_sort_hybrid(src, m + 1, r, dst, !srcToDst);      // reverse direction of srcToDst for the next level of recursion

        if (srcToDst) merge_parallel_L5(src, l, m, m + 1, r, dst, l);
        else          merge_parallel_L5(dst, l, m, m + 1, r, src, l);
    }

    template< class _Type >
    inline void inplace_merge_sort_hybrid(_Type* src, int l, int r, int threshold = 1024)
    {
        if (r <= l) {
            return;
        }
        if ((r - l) <= threshold) {
            std::sort(src + l, src + r + 1);    // could be insertion sort, with a smaller threshold
            return;
        }
        int m = ((r + l) / 2);

        inplace_merge_sort_hybrid(src, l,     m, threshold);
        inplace_merge_sort_hybrid(src, m + 1, r, threshold);

        std::inplace_merge(src + l, src + m + 1, src + r + 1);
    }

    template< class _Type >
    inline void parallel_inplace_merge_sort_hybrid_inner(_Type* src, size_t l, size_t r, size_t parallelThreshold = 1024)
    {
        if (r <= l) {
            return;
        }
        if ((r - l) <= parallelThreshold) {
            std::sort(src + l, src + r + 1);
            return;
        }
        size_t m = ((r + l) / 2);
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        Concurrency::parallel_invoke(
#else
        tbb::parallel_invoke(
#endif
            [&] { parallel_inplace_merge_sort_hybrid_inner(src, l, m, parallelThreshold); },
            [&] { parallel_inplace_merge_sort_hybrid_inner(src, m + 1, r, parallelThreshold); }
        );
        std::inplace_merge(std::execution::par_unseq, src + l, src + m + 1, src + r + 1);
    }

    template< class _Type >
    inline void parallel_inplace_merge_sort_hybrid(_Type* src, size_t l, size_t r, size_t parallelThreshold = 24 * 1024)
    {
        // may return 0 when not able to detect
        const auto processor_count = std::thread::hardware_concurrency();
        //printf("Number of cores = %u \n", processor_count);

        if ((parallelThreshold * processor_count) < (r - l + 1))
            parallelThreshold = (r - l + 1) / processor_count;

        parallel_inplace_merge_sort_hybrid_inner(src, l, r, parallelThreshold);
    }

    template< class _Type >
    inline void merge_sort_inplace(_Type* src, size_t l, size_t r)
    {
        if (r <= l) return;

        size_t m = l + ( r - l ) / 2;             // computes the average without overflow

        merge_sort_inplace(src, l,     m);
        merge_sort_inplace(src, m + 1, r);

        std::inplace_merge(src + l, src + m + 1, src + r + 1);
    }

    template< class _Type >
    inline void merge_sort_bottom_up_inplace(_Type* src, size_t l, size_t r)
    {
        for (size_t m = 1; m <= r - l; m = m + m)
            for (size_t i = l; i <= r - m; i += m + m)
                std::inplace_merge(src + i, src + i + m, src + __min(i + m + m, r + 1));
    }

}
#endif