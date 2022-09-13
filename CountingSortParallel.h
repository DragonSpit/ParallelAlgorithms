// TODO: Add a version of the Parallel Count/Histogram, which does not allocate the count array, but instead allocates a single array that's big enough to fit all of the needed count arrays
//       as a single buffer. A unique ID would also need to be provided to each of the leaf node (have done this before) to select a unique sub-buffer within the single buffer.
// TODO: Combine reading 64-bits technique with multi-buffering that removed dependency across for loop iterations into a single implementation to see if it performs even faster and more consistent.
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

//static void print_results_par(const char* const tag, high_resolution_clock::time_point startTime, high_resolution_clock::time_point endTime)
//{
//	printf("%s: Time: %fms\n", tag, duration_cast<duration<double, milli>>(endTime - startTime).count());
//}

namespace ParallelAlgorithms
{
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	template< unsigned long NumberOfBins >
	inline size_t* HistogramOneByteComponentParallel(unsigned char inArray[], size_t l, size_t r, size_t parallelThreshold = 64 * 1024)
	{
		size_t* countLeft  = NULL;
		size_t* countRight = NULL;

		if (l >= r)      // zero elements to compare
		{
			countLeft = new size_t[NumberOfBins]{};
			return countLeft;
		}
		if ((r - l) <= parallelThreshold)
		{
			countLeft = new size_t[NumberOfBins]{};

			for (size_t current = l; current < r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
				countLeft[inArray[current]]++;

			return countLeft;
		}

		size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;  // average without overflow

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { countLeft  = HistogramOneByteComponentParallel <NumberOfBins>(inArray, l, m, parallelThreshold); },
			[&] { countRight = HistogramOneByteComponentParallel <NumberOfBins>(inArray, m, r, parallelThreshold); }
		);
		// Combine left and right results
		for (size_t j = 0; j < NumberOfBins; j++)
			countLeft[j] += countRight[j];

		delete[] countRight;

		return countLeft;
	}

	// left (l) boundary is inclusive and right (r) boundary is exclusive
	template< unsigned long NumberOfBins >
	inline size_t* HistogramOneByteComponentParallel_4(unsigned char inArray[], size_t l, size_t r, size_t parallelThreshold = 64 * 1024)
	{
		size_t* countLeft  = NULL;
		size_t* countRight = NULL;

		if (l >= r)      // zero elements to compare
		{
			countLeft = new size_t[NumberOfBins]{};
			return countLeft;
		}
		if ((r - l) <= parallelThreshold)
		{
			countLeft = new size_t[NumberOfBins]{};
			size_t current = l;
			//if (((unsigned long long)(inArray + l) & 0x7) != 0)
			//	printf("Memory alignment is not on 8-byte boundary\n");
			// TODO: Detect not-64-bit aligned address and process bytes individually until alignment is achieved and then do 64-bits at a time
			size_t last_by_eight = l + ((r - l) / 8) * 8;
			unsigned long long* inArrayCurr = (unsigned long long*)(inArray + current);
			for (; current < last_by_eight; current += 8, inArrayCurr++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
			{
				unsigned long long eight_bytes = *inArrayCurr;
				countLeft[ eight_bytes        & 0xff]++;
				countLeft[(eight_bytes >>  8) & 0xff]++;
				countLeft[(eight_bytes >> 16) & 0xff]++;
				countLeft[(eight_bytes >> 24) & 0xff]++;
				countLeft[(eight_bytes >> 32) & 0xff]++;
				countLeft[(eight_bytes >> 40) & 0xff]++;
				countLeft[(eight_bytes >> 48) & 0xff]++;
				countLeft[(eight_bytes >> 56) & 0xff]++;
			}
			for (; current < r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
				countLeft[inArray[current]]++;

			return countLeft;
		}

		size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;  // average without overflow

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { countLeft  = HistogramOneByteComponentParallel_4 <NumberOfBins>(inArray, l, m, parallelThreshold); },
			[&] { countRight = HistogramOneByteComponentParallel_4 <NumberOfBins>(inArray, m, r, parallelThreshold); }
		);
		// Combine left and right results
		for (size_t j = 0; j < NumberOfBins; j++)
			countLeft[j] += countRight[j];

		delete[] countRight;

		return countLeft;
	}


	// left (l) boundary is inclusive and right (r) boundary is exclusive
	template< unsigned long numberOfBins >
	inline size_t* HistogramOneByteComponentParallel_2(unsigned char inArray[], size_t l, size_t r, size_t parallelThreshold = 64 * 1024)
	{
		size_t* countLeft_0 = NULL;
		size_t* countLeft_1 = NULL;
		size_t* countLeft_2 = NULL;
		size_t* countLeft_3 = NULL;
		size_t* countRight  = NULL;

		if (l >= r)      // zero elements to compare
		{
			countLeft_0 = new size_t[NumberOfBins]{};
			return countLeft_0;
		}
		if ((r - l) <= parallelThreshold)
		{
			countLeft_0 = new size_t[NumberOfBins]{};
			countLeft_1 = new size_t[NumberOfBins]{};
			countLeft_2 = new size_t[NumberOfBins]{};
			countLeft_3 = new size_t[NumberOfBins]{};

			size_t last_by_four = l + ((r - l) / 4) * 4;
			size_t current = l;
			for (; current < last_by_four;)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
			{
				countLeft_0[inArray[current]]++;  current++;
				countLeft_1[inArray[current]]++;  current++;
				countLeft_2[inArray[current]]++;  current++;
				countLeft_3[inArray[current]]++;  current++;
			}
			for (; current < r; current++)    // possibly last element
				countLeft_0[inArray[current]]++;

			// Combine the two count arrays into a single arrray to return
			for (size_t count_index = 0; count_index < numberOfBins; count_index++)
			{
				countLeft_0[count_index] += countLeft_1[count_index];
				countLeft_0[count_index] += countLeft_2[count_index];
				countLeft_0[count_index] += countLeft_3[count_index];
			}

			delete[] countLeft_3;
			delete[] countLeft_2;
			delete[] countLeft_1;
			return countLeft_0;
		}

		size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;  // average without overflow

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { countLeft_0 = HistogramOneByteComponentParallel_2 <NumberOfBins>(inArray, l, m, parallelThreshold); },
			[&] { countRight  = HistogramOneByteComponentParallel_2 <NumberOfBins>(inArray, m, r, parallelThreshold); }
		);
		// Combine left and right results
		for (size_t j = 0; j < NumberOfBins; j++)
			countLeft_0[j] += countRight[j];

		delete[] countRight;

		return countLeft_0;
	}

	// left (l) boundary is inclusive and right (r) boundary is exclusive
	template< unsigned long NumberOfBins >
	inline size_t* HistogramOneByteComponentParallel_3(unsigned char inArray[], size_t l, size_t r, size_t parallelThreshold = 64 * 1024)
	{
		size_t* countLeft_0 = NULL;
		size_t* countRight  = NULL;

		if (l >= r)      // zero elements to compare
		{
			countLeft_0 = new size_t[NumberOfBins]{};
			return countLeft_0;
		}
		if ((r - l) <= parallelThreshold)
		{
			countLeft_0 = new size_t[NumberOfBins]{};
			__declspec(align(64)) size_t countLeft_1[NumberOfBins] = { 0 };
			__declspec(align(64)) size_t countLeft_2[NumberOfBins] = { 0 };
			__declspec(align(64)) size_t countLeft_3[NumberOfBins] = { 0 };

			size_t last_by_four = l + ((r - l) / 4) * 4;
			size_t current = l;
			for (; current < last_by_four;)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
			{
				countLeft_0[inArray[current]]++;  current++;
				countLeft_1[inArray[current]]++;  current++;
				countLeft_2[inArray[current]]++;  current++;
				countLeft_3[inArray[current]]++;  current++;
			}
			for (; current < r; current++)    // possibly last element
				countLeft_0[inArray[current]]++;

			// Combine the two count arrays into a single arrray to return
			for (size_t count_index = 0; count_index < NumberOfBins; count_index++)
			{
				countLeft_0[count_index] += countLeft_1[count_index];
				countLeft_0[count_index] += countLeft_2[count_index];
				countLeft_0[count_index] += countLeft_3[count_index];
			}

			return countLeft_0;
		}

		size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;  // average without overflow

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { countLeft_0 = HistogramOneByteComponentParallel_3 <NumberOfBins>(inArray, l, m, parallelThreshold); },
			[&] { countRight  = HistogramOneByteComponentParallel_3 <NumberOfBins>(inArray, m, r, parallelThreshold); }
		);
		// Combine left and right results
		for (size_t j = 0; j < NumberOfBins; j++)
			countLeft_0[j] += countRight[j];

		delete[] countRight;

		return countLeft_0;
	}

	// left (l) boundary is inclusive and right (r) boundary is exclusive
	template< unsigned long NumberOfBins >
    inline void counting_sort_parallel_inner(unsigned char *array_to_sort, size_t l, size_t r, size_t threshold_count = 64 * 1024, size_t threshold_fill = 64 * 1024)
    {
		//const auto startTimeHistogram = high_resolution_clock::now();

		size_t* counts = HistogramOneByteComponentParallel_3< NumberOfBins >(array_to_sort, l, r, threshold_count);
		//size_t* counts = HistogramOneByteComponentParallel< NumberOfBins >(array_to_sort, l, r, threshold_count);

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
			//std::fill(array_to_sort + start_index, array_to_sort + start_index + counts[count_index], count_index);

			//size_t end_index = start_index + counts[count_index];		// for loop leads to 3X slower algorithm
			//for (size_t i = start_index; i <= end_index; i++)
			//	array_to_sort[i] = count_index;
            start_index += counts[count_index];
        }
#else
		size_t start_indexes[NumberOfBins];
		start_indexes[0] = 0;
		for (size_t count_index = 1; count_index < NumberOfBins; count_index++)
			start_indexes[count_index] = start_indexes[count_index - 1] + counts[count_index - 1];

		Concurrency::parallel_for(size_t(0), size_t(NumberOfBins), [&](size_t count_index) {
			parallel_fill(array_to_sort, (unsigned char)count_index, start_indexes[count_index], start_indexes[count_index] + counts[count_index], threshold_fill);
			});
#endif
		//const auto endTimeFill = high_resolution_clock::now();
		//print_results_par("Parallel Fill inside byte array Counting Sort", startTimeFill, endTimeFill);
		delete[] counts;
	}

	inline void counting_sort_parallel(unsigned char* a, size_t a_size)
	{
		if (a_size == 0)	return;

		const unsigned long NumberOfBins = 256;
		//const long threshold_count = a_size / 18;       // 18 cores on 24-core seems to lead to maximal performance. 12-cores seems to be a good value for 6-core CPU
		//const long threshold_fill  = a_size / 12;	    // 10-12 cores on 24-core seems to lead to maximal performance, with 24-cores slowing down by 2X. 2-cores seems to be the best value for 6-core CPU. Using std::fill is even better
		const long threshold_count = 64 * 1024;      // 18 cores on 24-core seems to lead to maximal performance. 12-cores seems to be a good value for 6-core CPU
		const long threshold_fill  = 64 * 1024;	    // 10-12 cores on 24-core seems to lead to maximal performance, with 24-cores slowing down by 2X. 2-cores seems to be the best value for 6-core CPU

        counting_sort_parallel_inner< NumberOfBins >(a, 0, a_size, threshold_count, threshold_fill);
		//counting_sort_parallel_inner< PowerOfTwoRadix >(a, 0, a_size);
	}
}
#endif