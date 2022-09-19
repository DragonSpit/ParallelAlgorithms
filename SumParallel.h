#pragma once

// Parallel Sum implementations

#ifndef _SumParallel_h
#define _SumParallel_h

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
#include <tbb/parallel_invoke.h>
#endif

#include "RadixSortMsdParallel.h"
#include "FillParallel.h"

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;
using std::random_device;
using std::sort;
using std::vector;


void print_results(const char* const tag, const unsigned long long sum, size_t sum_array_length,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime) {
	printf("%s: Sum: %llu   Array Length: %llu    Time: %fms\n", tag, sum, sum_array_length,
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

namespace ParallelAlgorithms
{
// left (l) boundary is inclusive and right (r) boundary is exclusive
inline unsigned long long Sum(unsigned long long in_array[], size_t l, size_t r)
{
	unsigned long long sum = 0;
	for (size_t current = l; current < r; current++)
		sum += in_array[current];
	//unsigned long long sum_left = std::accumulate(in_array + l, in_array + r, 0);	// may be implemented using SIMD/SSE
	return sum;
}

	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long SumParallel(unsigned long long in_array[], size_t l, size_t r, size_t parallelThreshold = 16 * 1024)
	{
		//if (((unsigned long long)(in_array + l) & 0x7) != 0)
		//	printf("Memory alignment is not on 8-byte boundary\n");
		if ((r - l) <= parallelThreshold)
			return Sum( in_array, l, r );

		unsigned long long sum_left = 0, sum_right = 0;

		size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;  // average without overflow

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { sum_left  = SumParallel(in_array, l, m, parallelThreshold); },
			[&] { sum_right = SumParallel(in_array, m, r, parallelThreshold); }
		);
		// Combine left and right results
		sum_left += sum_right;

		return sum_left;
	}
	// Sum of an arbitrary numerical type to a 64-bit sum
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	template< class _Type >
	inline long long SumParallel(_Type in_array[], size_t l, size_t r, size_t parallelThreshold = 16 * 1024)
	{
		//if (((unsigned long long)(in_array + l) & 0x7) != 0)
		//	printf("Memory alignment is not on 8-byte boundary\n");
		if ((r - l) <= parallelThreshold)
		{
			long long sum_left = 0;
			for (size_t current = l; current < r; current++)
				sum_left += (long long)in_array[current];
			//long long sum_left = std::accumulate(in_array + l, in_array + r, 0);
			return sum_left;
		}

		long long sum_left = 0, sum_right = 0;

		size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;  // average without overflow

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { sum_left  = SumParallel(in_array, l, m, parallelThreshold); },
			[&] { sum_right = SumParallel(in_array, m, r, parallelThreshold); }
		);
		// Combine left and right results
		sum_left += sum_right;

		return sum_left;
	}
}

#endif