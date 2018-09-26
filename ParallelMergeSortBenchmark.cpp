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

void print_results(const char *const tag, const double * sorted, size_t sortedLength,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime) {
	printf("%s: Lowest: %g Highest: %g Time: %fms\n", tag,
		sorted[0],sorted[sortedLength - 1],
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

int ParallelMergeSortBenchmark(vector<double>& doubles)
{
	random_device rd;

	// generate some random doubles:
	printf("\nBenchmarking Parallel Merge Sort Hybrid with %zu doubles...\n", testSize);
	double * doublesCopy  = new double [doubles.size()];

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (unsigned int j = 0; j < doubles.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			doublesCopy[j] = doubles[j];
		}
		double * sorted = new double [doubles.size()];
		const auto startTime = high_resolution_clock::now();
		parallel_merge_sort_hybrid_rh_1(doublesCopy, 0, (int)(doubles.size() - 1), sorted);	// ParallelMergeSort modifies the source array
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Merge Sort", sorted, doubles.size(), startTime, endTime);
		delete[] sorted;
	}

	delete [] doublesCopy;

	return 0;
}
