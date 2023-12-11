#include <stddef.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <ratio>
#include <vector>
#include <execution>

#include "RadixSortLSD.h"
#include "RadixSortLsdParallel.h"

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

int RadixSortLsdBenchmark(vector<unsigned>& uints)
{
	// generate some random uints:
	unsigned* uintsCopy = new unsigned[uints.size()];
	//unsigned long* uintsCopy = (unsigned long*) operator new[](sizeof(unsigned long) * uints.size(), (std::align_val_t)(128));
	unsigned* sorted = new unsigned[uints.size()];
	//unsigned long* tmp_working = (unsigned long*) operator new[](sizeof(unsigned long) * uints.size(), (std::align_val_t)(128));

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (unsigned int j = 0; j < uints.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			uintsCopy[j] = uints[j];
			sorted[j] = j;									// page in the destination array into system memory
		}
		// Eliminate compiler ability to optimize paging-in of the input and output arrays
		// Paging-in source and destination arrays leads to a 50% speed-up on Linux, and 15% on Windows

		vector<unsigned> sorted_reference(uints);
		sort(sorted_reference.begin(), sorted_reference.end());

		//printf("uintsCopy address = %p   sorted address = %p   value at a random location = %lu %lu\n", uintsCopy, sorted, sorted[static_cast<unsigned>(rd()) % uints.size()], uintsCopy[static_cast<unsigned>(rd()) % uints.size()]);
		const auto startTime = high_resolution_clock::now();
		//RadixSortLSDPowerOf2Radix_unsigned_TwoPhase(uintsCopy, sorted, (unsigned long)uints.size());
		//sort_radix_in_place_adaptive(uintsCopy, (unsigned long)uints.size(), 0.1);
		//sort_radix_in_place_stable_adaptive(uintsCopy, uints.size(), 0.9);

		RadixSortLSDPowerOf2Radix_unsigned_TwoPhase_DeRandomize(uintsCopy, sorted, (unsigned long)uints.size());
		//RadixSortLSDPowerOf2RadixParallel_unsigned_TwoPhase(uintsCopy, sorted, (unsigned long)uints.size());
		const auto endTime = high_resolution_clock::now();
		print_results("Radix Sort LSD", sorted, uints.size(), startTime, endTime);
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

int ParallelRadixSortLsdBenchmark(vector<unsigned>& uints)
{
	// generate some random uints:
	//unsigned long* uintsCopy = new unsigned long[uints.size()];
	//unsigned* uintsCopy = static_cast<unsigned*>(operator new[](sizeof(unsigned) * uints.size(), (std::align_val_t)(128)));
	vector<unsigned> uintsCopy(uints.size());
	//unsigned long* uintsCopy = new(std::align_val_t{ 128 }) unsigned long[uints.size()];
	//unsigned long* uintsCopy  = (unsigned long*) operator new[](sizeof(unsigned long) * uints.size(), (std::align_val_t)(128));
	//unsigned long* tmp_working = new(std::align_val_t{ 128 }) unsigned long[uints.size()];
	//unsigned long* tmp_working = new unsigned long[uints.size()];
	//unsigned* tmp_working = static_cast<unsigned*>(operator new[](sizeof(unsigned) * uints.size(), (std::align_val_t)(128)));
	vector<unsigned> tmp_working(uints.size());

	printf("\n");
	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (unsigned int j = 0; j < uints.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			uintsCopy[j] = uints[j];
			tmp_working[j] = j;								// page in the destination array into system memory
		}
		// Eliminate compiler ability to optimize paging-in of the input and output arrays
		// Paging-in source and destination arrays leads to a 50% speed-up on Linux, and 15% on Windows

		vector<unsigned> sorted_reference(uints);
		stable_sort(std::execution::par_unseq, sorted_reference.begin(), sorted_reference.end());

		//printf("uintsCopy address = %p   sorted address = %p   value at a random location = %lu %lu\n", uintsCopy, tmp_working, tmp_working[static_cast<unsigned>(rd()) % uints.size()], uintsCopy[static_cast<unsigned>(rd()) % uints.size()]);
		const auto startTime = high_resolution_clock::now();
		//RadixSortLSDPowerOf2Radix_unsigned_TwoPhase(uintsCopy, tmp_working, uints.size());
		//RadixSortLSDPowerOf2RadixParallel_unsigned_TwoPhase_DeRandomize(uintsCopy, tmp_working, (unsigned long)uints.size());
		//SortRadixPar(uintsCopy, tmp_working, uints.size(), uints.size() / 24);		// slower than using all cores
		SortRadixPar(uintsCopy.data(), tmp_working.data(), uints.size());		// fastest on 96-core Intel and AMD AWS c7 nodes
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Radix Sort LSD", uintsCopy.data(), uints.size(), startTime, endTime);
		if (!std::equal(sorted_reference.begin(), sorted_reference.end(), uintsCopy.data()))
		{
			printf("Arrays are not equal\n");
			exit(1);
		}
	}

	//delete[] tmp_working;
	//::operator delete[](tmp_working, std::align_val_t{ 128 });
	//::operator delete[](new(std::align_val_t{ 128 }) unsigned long[uints.size()], std::align_val_t{ 128 });
	//delete[](operator new[](sizeof(unsigned long) * uints.size(), (std::align_val_t)(128)));
	//delete[] uintsCopy;
	//::operator delete[](uintsCopy, std::align_val_t{ 128 });
	//::operator delete[](new(std::align_val_t{ 128 }) unsigned long[uints.size()], std::align_val_t{ 128 });
	//::operator delete[](uintsCopy, std::align_val_t{ 128 });
	//delete[](operator new[](sizeof(unsigned long) * uints.size(), (std::align_val_t)(128)));

	return 0;
}
