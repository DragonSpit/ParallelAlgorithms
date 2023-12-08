#include <stddef.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <ratio>
#include <vector>
#include <execution>

#include "RadixSortMSD.h"
#include "RadixSortMsdParallel.h"

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;
using std::random_device;
using std::sort;
using std::vector;

const int iterationCount = 5; 

static void print_results(const char* const tag, const unsigned* sorted, size_t sortedLength,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime) {
	printf("%s: Lowest: %u Highest: %u Time: %fms\n", tag,
		sorted[0], sorted[sortedLength - 1],
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}


int RadixSortMsdBenchmark(vector<unsigned>& uints)
{
	// generate some random uints:
	unsigned* uintsCopy = new unsigned[uints.size()];
	//unsigned long* uintsCopy = (unsigned long*) operator new[](sizeof(unsigned long) * uints.size(), (std::align_val_t)(128));
	unsigned* sorted = new unsigned[uints.size()];
	//unsigned* tmp_working = (unsigned*) operator new[](sizeof(unsigned) * uints.size(), (std::align_val_t)(128));

	printf("\n");
	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (unsigned int j = 0; j < uints.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			//uints[j] = j + 2;							// for pre-sorted array testing
			uintsCopy[j] = uints[j];
			sorted[j] = j;									// page in the destination array into system memory
		}
		// Eliminate compiler ability to optimize paging-in of the input and output arrays
		// Paging-in source and destination arrays leads to a 50% speed-up on Linux, and 15% on Windows

		vector<unsigned> sorted_reference(uints);
		sort(sorted_reference.begin(), sorted_reference.end());

		//printf("uintsCopy address = %p   sorted address = %p   value at a random location = %lu %lu\n", uintsCopy, sorted, sorted[static_cast<unsigned>(rd()) % uints.size()], uintsCopy[static_cast<unsigned>(rd()) % uints.size()]);
		const auto startTime = high_resolution_clock::now();
		//RadixSortLSDPowerOf2RadixScalar_unsigned_TwoPhase(uintsCopy, sorted, (unsigned long)uints.size());
		//RadixSortLSDPowerOf2RadixParallel_unsigned_TwoPhase(uintsCopy, sorted, (unsigned long)uints.size());
		//hybrid_inplace_msd_radix_sort(uintsCopy, (unsigned long)uints.size());
		parallel_hybrid_inplace_msd_radix_sort(uintsCopy, (unsigned long)uints.size());
		//RadixSortMSDStablePowerOf2Radix_unsigned(uintsCopy, tmp_working, (unsigned long)uints.size());
		const auto endTime = high_resolution_clock::now();
		print_results("Radix Sort MSD", uintsCopy, uints.size(), startTime, endTime);
		if (!std::equal(sorted_reference.begin(), sorted_reference.end(), uintsCopy))
		{
			printf("Arrays are not equal\n");
			exit(1);
		}
	}

	delete[] sorted;
	//delete[](operator new[](sizeof(intptr_t) * uints.size(), (std::align_val_t)(64)));
	delete [] uintsCopy;
	//delete[](operator new[](sizeof(intptr_t) * uints.size(), (std::align_val_t)(64)));

	return 0;
}
