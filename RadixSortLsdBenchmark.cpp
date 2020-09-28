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

const int iterationCount = 5; 

extern void print_results(const char* const tag, const unsigned long* sorted, size_t sortedLength,
	                      high_resolution_clock::time_point startTime, high_resolution_clock::time_point endTime);

int RadixSortLsdBenchmark(vector<unsigned long>& ulongs)
{
	random_device rd;

	// generate some random ulongs:
	unsigned long* ulongsCopy = new unsigned long[ulongs.size()];
	//unsigned long* ulongsCopy = (unsigned long*) operator new[](sizeof(unsigned long) * ulongs.size(), (std::align_val_t)(128));
	unsigned long* sorted = new unsigned long[ulongs.size()];
	//unsigned long* sorted     = (unsigned long*) operator new[](sizeof(unsigned long) * ulongs.size(), (std::align_val_t)(128));

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (unsigned int j = 0; j < ulongs.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			ulongsCopy[j] = ulongs[j];
			sorted[j] = j;									// page in the destination array into system memory
		}
		// Eliminate compiler ability to optimize paging-in of the input and output arrays
		// Paging-in source and destination arrays leads to a 50% speed-up on Linux, and 15% on Windows
		printf("ulongsCopy address = %p   sorted address = %p   value at a random location = %lu %lu\n", ulongsCopy, sorted, sorted[static_cast<unsigned>(rd()) % ulongs.size()], ulongsCopy[static_cast<unsigned>(rd()) % ulongs.size()]);
		const auto startTime = high_resolution_clock::now();
		RadixSortLSDPowerOf2RadixScalar_unsigned_TwoPhase(ulongsCopy, sorted, (unsigned long)ulongs.size());
		const auto endTime = high_resolution_clock::now();
		print_results("Radix Sort LSD", sorted, ulongs.size(), startTime, endTime);
	}

	delete[] sorted;
	//delete[](operator new[](sizeof(intptr_t) * ulongs.size(), (std::align_val_t)(64)));
	delete [] ulongsCopy;
	//delete[](operator new[](sizeof(intptr_t) * ulongs.size(), (std::align_val_t)(64)));

	return 0;
}
