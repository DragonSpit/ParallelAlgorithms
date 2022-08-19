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

    template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, long Threshold >
    inline void counting_sort_parallel_inner(unsigned char *array_to_sort, size_t l, size_t r)
    {
        size_t* counts = HistogramOneByteComponentParallel< PowerOfTwoRadix, Log2ofPowerOfTwoRadix >(array_to_sort, l, r);

        size_t start_index = 0;
        for (size_t count_index = 0; count_index < PowerOfTwoRadix; count_index++)
        {
            // TODO: Fill needs to be a parallel function to be done in parallel, with each value done in parallel and within each value as well
            // Then use both of these combined to show that full memory bandwidth can be achieved in C# and C++. In C++ we can use memset() to do SSE
			if (counts[count_index] > 0)
			{
				parallel_fill_r(array_to_sort, (unsigned char)count_index, start_index, start_index + counts[count_index] - 1, (size_t)Threshold);
				//memset(array_to_sort + start_index, count_index, counts[count_index]);

				//size_t end_index = start_index + counts[count_index];		// for loop leads to 3X slower algorithm
				//for (size_t i = start_index; i <= end_index; i++)
				//	array_to_sort[i] = count_index;
			}
            start_index += counts[count_index];
        }
    }

	inline void counting_sort_parallel(unsigned char* a, std::size_t a_size)
	{
		if (a_size == 0)	return;

		const unsigned long PowerOfTwoRadix = 256;
		const unsigned long Log2ofPowerOfTwoRadix = 8;
		const long Threshold = 16 * 1024;

        counting_sort_parallel_inner< PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(a, (std::size_t)0, (std::size_t)(a_size - 1));
	}
}
#endif