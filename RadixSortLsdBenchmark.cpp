#include <stddef.h>
#include <stdio.h>
#include <algorithm>
#include <chrono>
#include <random>
#include <ratio>
#include <vector>
#include <execution>
#include "RadixSortLSD.h"

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;
using std::random_device;
using std::sort;
using std::vector;

const size_t testSize = 1'000'000;
const int iterationCount = 5; 

void print_results(const char *const tag, const unsigned long * sorted, size_t sortedLength,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime) {
	printf("%s: Lowest: %u Highest: %u Time: %fms\n", tag,
		sorted[0], sorted[sortedLength - 1],
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

int RadixSortLsdBenchmark(vector<unsigned long>& ulongs)
{
	random_device rd;

	// generate some random ulongs:
	printf("\nBenchmarking Radix Sort LSD with %zu unsigned longs...\n", testSize);
	unsigned long * ulongsCopy  = new unsigned long [ulongs.size()];

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (unsigned int j = 0; j < ulongs.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			ulongsCopy[j] = ulongs[j];
		}
		unsigned long * sorted = new unsigned long [ulongs.size()];
		const auto startTime = high_resolution_clock::now();
		RadixSortLSDPowerOf2RadixScalar_unsigned_TwoPhase(ulongsCopy, sorted, (unsigned long)ulongs.size());
		const auto endTime = high_resolution_clock::now();
		print_results("Radix Sort LSD", sorted, ulongs.size(), startTime, endTime);
		delete[] sorted;
	}

	delete [] ulongsCopy;

	return 0;
}
