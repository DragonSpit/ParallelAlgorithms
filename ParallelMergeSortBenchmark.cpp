#include <stddef.h>
#include <stdio.h>
#include <algorithm>
#include <chrono>
#include <random>
#include <ratio>
#include <vector>
#include <execution>

#include "ParallelMergeSort.h"

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;
using std::random_device;
using std::sort;
using std::vector;

const int iterationCount = 5; 

void print_results(const char *const tag, const double * sorted, size_t sortedLength,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime) {
	printf("%s: Lowest: %g Highest: %g Time: %fms\n", tag,
		sorted[0], sorted[sortedLength - 1],
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

void print_results(const char* const tag, const unsigned long* sorted, size_t sortedLength,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime) {
	printf("%s: Lowest: %lu Highest: %lu Time: %fms\n", tag,
		sorted[0], sorted[sortedLength - 1],
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

void print_results(const char* const tag, const unsigned* sorted, size_t sortedLength,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime) {
	printf("%s: Lowest: %u Highest: %u Time: %fms\n", tag,
		sorted[0], sorted[sortedLength - 1],
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

int ParallelMergeSortBenchmark(vector<double>& doubles)
{
	random_device rd;

	// generate some random uints:
	printf("\nBenchmarking Parallel Merge Sort Hybrid with %zu doubles...\n", doubles.size());
	double * doublesCopy  = new double [doubles.size()];
	double* sorted = new double[doubles.size()];

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (unsigned int j = 0; j < doubles.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			doublesCopy[j] = doubles[j];
			sorted[j] = j;									// page in the destination array into system memory
		}
		const auto startTime = high_resolution_clock::now();
		parallel_merge_sort_hybrid_rh_1(doublesCopy, 0, (int)(doubles.size() - 1), sorted);	// ParallelMergeSort modifies the source array
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Merge Sort", sorted, doubles.size(), startTime, endTime);
	}

	delete[] sorted;
	delete[] doublesCopy;

	return 0;
}

int ParallelMergeSortBenchmark(vector<unsigned long>& ulongs)
{
	random_device rd;

	// generate some random uints:
	printf("\nBenchmarking Parallel Merge Sort Hybrid with %zu unsigned longs (each of %llu bytes)...\n", ulongs.size(), sizeof(unsigned long));
	unsigned long* ulongsCopy = new unsigned long[ulongs.size()];
	unsigned long* sorted = new unsigned long[ulongs.size()];

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (unsigned int j = 0; j < ulongs.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			ulongsCopy[j] = ulongs[j];
			sorted[j] = j;									// page in the destination array into system memory
		}
		const auto startTime = high_resolution_clock::now();
		parallel_merge_sort_hybrid_rh_1(ulongsCopy, 0, (int)(ulongs.size() - 1), sorted);	// ParallelMergeSort modifies the source array
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Merge Sort", sorted, ulongs.size(), startTime, endTime);
	}

	delete[] sorted;
	delete[] ulongsCopy;

	return 0;
}

int ParallelMergeSortBenchmark(vector<unsigned>& uints)
{
	random_device rd;

	// generate some random uints:
	printf("\nBenchmarking Parallel Merge Sort Hybrid with %zu unsigned integers (each of %llu bytes)...\n", uints.size(), sizeof(unsigned));
	unsigned* uintsCopy = new unsigned[uints.size()];
	unsigned* sorted = new unsigned[uints.size()];

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (unsigned int j = 0; j < uints.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			uintsCopy[j] = uints[j];
			sorted[j] = j;									// page in the destination array into system memory
		}
		const auto startTime = high_resolution_clock::now();
		//parallel_merge_sort_hybrid_rh_1(uintsCopy, 0, (int)(uints.size() - 1), sorted);	// ParallelMergeSort modifies the source array
		parallel_merge_sort_hybrid(uintsCopy, 0, (int)(uints.size() - 1), sorted);	// ParallelMergeSort modifies the source array
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Merge Sort", sorted, uints.size(), startTime, endTime);
	}

	delete[] sorted;
	delete[] uintsCopy;

	return 0;
}
