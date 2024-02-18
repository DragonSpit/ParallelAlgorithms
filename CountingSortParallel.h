// TODO: Add a version of the Parallel Count/Histogram, which does not allocate the count array, but instead allocates a single array that's big enough to fit all of the needed count arrays
//       as a single buffer. A unique ID would also need to be provided to each of the leaf node (have done this before) to select a unique sub-buffer within the single buffer.
// TODO: Combine reading 64-bits technique with multi-buffering that removed dependency across for loop iterations into a single implementation to see if it performs even faster and more consistent.
#pragma once

// Parallel Counting Sort implementations

#ifndef _ParallelCountingSort_h
#define _ParallelCountingSort_h

#include "Configuration.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <ratio>
#include <vector>
#include <thread>
#include <execution>

#include "RadixSortMsdParallel.h"
#include "FillParallel.h"
#include "HistogramParallel.h"

namespace ParallelAlgorithms
{
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
			//std::fill(oneapi::dpl::execution::par_unseq, array_to_sort + start_index, array_to_sort + start_index + counts[count_index], count_index);

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

#if defined(USE_PPL)
		Concurrency::parallel_for(size_t(0), size_t(NumberOfBins), [&](size_t count_index)
#else
		tbb::parallel_for(size_t(0), size_t(NumberOfBins), [&](size_t count_index)
#endif
		{
			parallel_fill(array_to_sort, (unsigned char)count_index, start_indexes[count_index], start_indexes[count_index] + counts[count_index], threshold_fill);
			//std::fill(oneapi::dpl::execution::par_unseq, array_to_sort + start_indexes[count_index], array_to_sort + start_indexes[count_index] + counts[count_index], count_index);
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