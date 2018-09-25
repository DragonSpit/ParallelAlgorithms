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

const size_t testSize = 1'000'000;
const int iterationCount = 5; 

void print_results(const char *const tag, const double * sorted, unsigned int sortedLength,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime) {
	printf("%s: Lowest: %g Highest: %g Time: %fms\n", tag,
		sorted[0],sorted[sortedLength - 1],
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

int ParallelMergeSortBenchmark()
{
	random_device rd;

	// generate some random doubles:
	printf("\nBenchmarking Parallel Merge Sort Hybrid with %zu doubles...\n", testSize);
	double * doubles = new double [testSize];

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (unsigned int j = 0; j < testSize; j++) {	// generate new source array, since ParallelMergeSort modifies the source each time it's used
			doubles[j] = static_cast<double>(rd());
		}
		double * sorted = new double [testSize];
		const auto startTime = high_resolution_clock::now();
		parallel_merge_sort_hybrid_rh_1(doubles, 0, (int)(testSize - 1), sorted);
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Merge Sort", sorted, testSize, startTime, endTime);
		delete[] sorted;
	}

	delete [] doubles;

	return 0;
}
