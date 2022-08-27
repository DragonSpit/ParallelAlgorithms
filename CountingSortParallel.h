// TODO: Add a version of the Parallel Count/Histogram, which does not allocate the count array, but instead allocates a single array that's big enough to fit all of the needed count arrays
//       as a single buffer. A unique ID would also need to be provided to each of the leaf node (have done this before) to select a unique sub-buffer within the single buffer.
#pragma once

// Parallel Counting Sort implementations

#ifndef _ParallelCountingSort_h
#define _ParallelCountingSort_h

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

#include "RadixSortMsdParallel.h"
#include "FillParallel.h"

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;
using std::random_device;
using std::sort;
using std::vector;

void print_results_par(const char* const tag, high_resolution_clock::time_point startTime, high_resolution_clock::time_point endTime)
{
	printf("%s: Time: %fms\n", tag, duration_cast<duration<double, milli>>(endTime - startTime).count());
}

namespace ParallelAlgorithms
{
	template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
	inline size_t* HistogramOneByteComponentParallel(unsigned char inArray[], size_t l, size_t r, size_t parallelThreshold = 16 * 1024)
	{
		const unsigned long numberOfBins = PowerOfTwoRadix;

		size_t* countLeft  = NULL;
		size_t* countRight = NULL;

		if (l > r)      // zero elements to compare
		{
			countLeft = new size_t[numberOfBins];
			for (size_t j = 0; j < numberOfBins; j++)
				countLeft[j] = 0;
			return countLeft;
		}
		if ((r - l + 1) <= parallelThreshold)
		{
			countLeft = new size_t[numberOfBins];
			for (size_t j = 0; j < numberOfBins; j++)
				countLeft[j] = 0;

			for (size_t current = l; current <= r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
				countLeft[inArray[current]]++;

			return countLeft;
		}

		size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;  // average without overflow

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { countLeft  = HistogramOneByteComponentParallel <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l,     m, parallelThreshold); },
			[&] { countRight = HistogramOneByteComponentParallel <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, m + 1, r, parallelThreshold); }
		);
		// Combine left and right results
		for (size_t j = 0; j < numberOfBins; j++)
			countLeft[j] += countRight[j];

		delete[] countRight;

		return countLeft;
	}

    template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
    inline void counting_sort_parallel_inner(unsigned char *array_to_sort, size_t l, size_t r, size_t threshold_count = 16 * 1024, size_t threshold_fill = 16 * 1024)
    {
		//const auto startTimeHistogram = high_resolution_clock::now();

		size_t* counts = HistogramOneByteComponentParallel< PowerOfTwoRadix, Log2ofPowerOfTwoRadix >(array_to_sort, l, r, threshold_count);

		//const auto endTimeHistogram = high_resolution_clock::now();
		//print_results_par("Parallel Histogram inside byte array Counting Sort", startTimeHistogram, endTimeHistogram);

		//const auto startTimeFill = high_resolution_clock::now();
#if 0
		size_t start_index = 0;
        for (size_t count_index = 0; count_index < PowerOfTwoRadix; count_index++)
        {
            // Then use both of these combined to show that full memory bandwidth can be achieved in C# and C++. In C++ we can use memset() to do SSE
			parallel_fill(array_to_sort, (unsigned char)count_index, start_index, start_index + counts[count_index], threshold_fill);
			//parallel_fill_2(array_to_sort, (unsigned char)count_index, start_index, counts[count_index], threshold_fill);
			//memset(array_to_sort + start_index, count_index, counts[count_index]);

			//size_t end_index = start_index + counts[count_index];		// for loop leads to 3X slower algorithm
			//for (size_t i = start_index; i <= end_index; i++)
			//	array_to_sort[i] = count_index;
            start_index += counts[count_index];
        }
#else
		size_t start_indexes[PowerOfTwoRadix];
		start_indexes[0] = 0;
		for (size_t count_index = 1; count_index < PowerOfTwoRadix; count_index++)
			start_indexes[count_index] = start_indexes[count_index - 1] + counts[count_index - 1];

		Concurrency::parallel_for(size_t(0), size_t(PowerOfTwoRadix), [&](size_t count_index) {
			parallel_fill(array_to_sort, (unsigned char)count_index, start_indexes[count_index], start_indexes[count_index] + counts[count_index], threshold_fill);
			});
#endif
		//const auto endTimeFill = high_resolution_clock::now();
		//print_results_par("Parallel Fill inside byte array Counting Sort", startTimeFill, endTimeFill);
	}

	inline void counting_sort_parallel(unsigned char* a, std::size_t a_size)
	{
		if (a_size == 0)	return;

		const unsigned long PowerOfTwoRadix = 256;
		const unsigned long Log2ofPowerOfTwoRadix = 8;
		const long threshold_count = a_size / 24;
		const long threshold_fill  = 64 * 1024;

        counting_sort_parallel_inner< PowerOfTwoRadix, Log2ofPowerOfTwoRadix >(a, (std::size_t)0, (std::size_t)(a_size - 1), threshold_count, threshold_fill);
	}
}
#endif