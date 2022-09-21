#include <stddef.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <ratio>
#include <vector>
#include <execution>

#include "CountingSortParallel.h"
#include "CountingSort.h"

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;
using std::random_device;
using std::sort;
using std::vector;

const int iterationCount = 5;

extern void print_results(const char* const tag, const unsigned char* sorted, size_t sortedLength,
	high_resolution_clock::time_point startTime, high_resolution_clock::time_point endTime);

int CountingSortBenchmark(vector<unsigned long>& ulongs)
{
	unsigned char* ucharCopy = new unsigned char[ulongs.size()];
	unsigned char* sorted    = new unsigned char[ulongs.size()];
	unsigned long long* u64array = new unsigned long long[ulongs.size()];

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (size_t j = 0; j < ulongs.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			//ulongs[j] = j + 2;							// for pre-sorted array testing
			ucharCopy[j] = (unsigned char)ulongs[j];
			sorted[j]    = (unsigned char)j;				// page in the destination array into system memory
			u64array[j]  = (unsigned long long)j;
		}
		// Eliminate compiler ability to optimize paging-in of the input and output arrays
		// Paging-in source and destination arrays leads to a 50% speed-up on Linux, and 15% on Windows

		vector<unsigned char> sorted_reference(ulongs.size());
		for (size_t j = 0; j < ulongs.size(); j++)
			sorted_reference[j] = (unsigned char)ulongs[j];
		const auto startTimeRef = high_resolution_clock::now();
		//sort(sorted_reference.begin(), sorted_reference.end());
		sort(std::execution::par_unseq, sorted_reference.begin(), sorted_reference.end());
		//sort(oneapi::dpl::execution::par_unseq, sorted_reference.begin(), sorted_reference.end());
		const auto endTimeRef = high_resolution_clock::now();
		print_results("std::sort of byte array", ucharCopy, ulongs.size(), startTimeRef, endTimeRef);

		//printf("ulongsCopy address = %p   sorted address = %p   value at a random location = %lu %lu\n", ucharCopy, sorted, sorted[static_cast<unsigned>(rd()) % ulongs.size()], ulongsCopy[static_cast<unsigned>(rd()) % ulongs.size()]);
		const auto startTime = high_resolution_clock::now();
		//ParallelAlgorithms::counting_sort(ucharCopy, 0, ulongs.size());
		ParallelAlgorithms::counting_sort_parallel(ucharCopy, ulongs.size());
		//ParallelAlgorithms::parallel_fill<unsigned long long>(u64array, 0, 0, ulongs.size(), ulongs.size() / 24);	// same performance for filling array of 64-bit
		//ParallelAlgorithms::parallel_fill<unsigned char>(sorted, 0, 0, ulongs.size(), ulongs.size() / 2);        // dividing by # cores provides more consistent performance
		//std::fill(std::execution::par_unseq, ucharCopy + 0, ucharCopy + ulongs.size(), 10);						// does not support parallel
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Counting Sort", ucharCopy, ulongs.size(), startTime, endTime);
		if (std::equal(sorted_reference.begin(), sorted_reference.end(), ucharCopy))
			printf("Arrays are equal\n");
		else
		{
			printf("Arrays are not equal\n");
			exit(1);
		}
	}

	delete[] sorted;
	delete[] ucharCopy;

	return 0;
}

// Test memory allocation
int TestMemoryAllocation()
{
	const size_t NUM_TIMES = 1000;
	const size_t SIZE_OF_ARRAY = 100'000'000;
	unsigned char* array_of_pointers[NUM_TIMES]{};
	size_t sum = 0;

	for (size_t i = 0; i < NUM_TIMES; ++i)
	{
		array_of_pointers[i] = new unsigned char[SIZE_OF_ARRAY];
		for (size_t j = 0; j < SIZE_OF_ARRAY; ++j)
			array_of_pointers[i][j] = (unsigned char)j;
		for (size_t j = 0; j < SIZE_OF_ARRAY; ++j)
			sum += array_of_pointers[i][j];
		printf("Allocated array: %zu   sum = %zu\n", i, sum);
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}
	printf("Final sum = %zu\n", sum);

	for (size_t i = 0; i < NUM_TIMES; ++i)
	{
		delete[] array_of_pointers[i];
	}

	return 0;
}

