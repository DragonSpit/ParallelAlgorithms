//#include <oneapi/dpl/execution>
//#include <oneapi/dpl/algorithm>
#include <stddef.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <ratio>
#include <vector>
//#include <execution>
//#include <oneapi/dpl/algorithm>
//#define __TBB_PREVIEW_TASK_ARENA_CONSTRAINTS_EXTENSION_PRESENT 1
//#include <oneapi/tbb/task_arena.h>

#include "SumParallel.h"

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;
using std::random_device;
using std::sort;
using std::vector;

const int iterationCount = 100;

extern void print_results(const char* const tag, const unsigned long long sum, size_t sum_array_length,
	high_resolution_clock::time_point startTime, high_resolution_clock::time_point endTime);


int SumBenchmarkChar(vector<unsigned long>& ulongs)
{
	vector<unsigned char> u8Copy(ulongs.size());
	unsigned char *u8Array = new unsigned char[ulongs.size()];

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (size_t j = 0; j < ulongs.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			u8Array[j] = (unsigned char)ulongs[j];
			u8Copy[ j] = (unsigned char)ulongs[j];
		}
		// Eliminate compiler ability to optimize paging-in of the input and output arrays
		// Paging-in source and destination arrays leads to a 50% speed-up on Linux, and 15% on Windows

		const auto startTimeRef = high_resolution_clock::now();
		long long sum_ref = 0;
		for (size_t i = 0; i < ulongs.size(); i++)
			sum_ref += u8Copy[i];
		//sum_ref = std::accumulate(u8Copy.begin(), u8Copy.end(), 0);
		const auto endTimeRef = high_resolution_clock::now();
		print_results("std::accumulate", sum_ref, u8Copy.size(), startTimeRef, endTimeRef);

		long long sum = 0;
		for (size_t k = 0; k < 100; k++)
		{
			const auto startTime = high_resolution_clock::now();
			//long long sum = ParallelAlgorithms::SumParallel(u8Array, 0, ulongs.size());
			//sum = ParallelAlgorithms::SumParallel(u8Array, 0, ulongs.size(), ulongs.size() / 24);	// Running on 24-core is fastest, however with 2.7X run-to-run variation
			sum = ParallelAlgorithms::SumParallel(u8Array, 0, ulongs.size());
			const auto endTime = high_resolution_clock::now();
			print_results("Parallel Sum", sum, ulongs.size(), startTime, endTime);
		}
		if (sum == sum_ref)
			printf("Sums are equal\n");
		else
		{
			printf("Sums are not equal\n");
			exit(1);
		}
	}
	return 0;
}
int SumBenchmark(vector<unsigned long>& ulongs)
{
	vector<unsigned long long> u64Copy(ulongs.size());
	unsigned long long* u64Array = new unsigned long long[ulongs.size()];

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (size_t j = 0; j < ulongs.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			u64Array[j] = (unsigned long long)ulongs[j];
			u64Copy[j] = (unsigned long long)ulongs[j];
		}
		// Eliminate compiler ability to optimize paging-in of the input and output arrays
		// Paging-in source and destination arrays leads to a 50% speed-up on Linux, and 15% on Windows

		const auto startTimeRef = high_resolution_clock::now();
		unsigned long long sum_ref = 0;
		for (size_t i = 0; i < ulongs.size(); i++)
			sum_ref += u64Copy[i];
		//sum_ref = std::accumulate(u64Copy.begin(), u64Copy.end(), 0);
		//std::fill(oneapi::dpl::execution::par_unseq, u64Copy.begin(), u64Copy.end(), 42);
		//std::fill(u64Copy.begin(), u64Copy.end(), 42);
		const auto endTimeRef = high_resolution_clock::now();
		print_results("std::accumulate", sum_ref, u64Copy.size(), startTimeRef, endTimeRef);

		const auto startTime = high_resolution_clock::now();
		//unsigned long long sum = ParallelAlgorithms::SumParallel(u64Array, 0, ulongs.size());	// Running on 24-core is fastest, however with 2.7X run-to-run variation
		//unsigned long long sum = ParallelAlgorithms::SumParallel(u64Array, 0, ulongs.size(), ulongs.size() / 24);	// Running on 24-core is fastest, however with 2.7X run-to-run variation
		//unsigned long long sum = ParallelAlgorithms::SumParallelNonRecursive(u64Array, 0, ulongs.size());
		//unsigned long long sum = ParallelAlgorithms::SumParallelNonRecursive(u64Array, 0, ulongs.size(), ulongs.size() / 8);
		unsigned long long sum = 0;
		for (int j = 0; j < 100; ++j)
		{
			//sum = ParallelAlgorithms::SumParallelNonRecursiveNoHyperthreading(u64Array, 0, ulongs.size(), ulongs.size() / 16);
			//sum = ParallelAlgorithms::SumNonRecursive(u64Array, 0, ulongs.size());
			//sum = ParallelAlgorithms::SumParallelNonRecursive(u64Array, 0, ulongs.size(), ulongs.size() / 32);
			sum = ParallelAlgorithms::SumParallel(u64Array, 0, ulongs.size(), ulongs.size() / 16);	// highest performance with /15 and /17 at half the performance
		}
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Sum", sum, ulongs.size(), startTime, endTime);
		if (sum == sum_ref)
			printf("Sums are equal\n");
		else
		{
			printf("Sums are not equal\n");
			exit(1);
		}
	}
	return 0;
}
