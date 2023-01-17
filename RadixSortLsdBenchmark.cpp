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

extern void print_results(const char* const tag, const unsigned long* sorted, size_t sortedLength,
	                      high_resolution_clock::time_point startTime, high_resolution_clock::time_point endTime);

int RadixSortLsdBenchmark(vector<unsigned long>& ulongs)
{
	random_device rd;

	// generate some random ulongs:
	unsigned long* ulongsCopy = new unsigned long[ulongs.size()];
	//unsigned long* ulongsCopy = (unsigned long*) operator new[](sizeof(unsigned long) * ulongs.size(), (std::align_val_t)(128));
	unsigned long* sorted = new unsigned long[ulongs.size()];
	//unsigned long* tmp_working     = (unsigned long*) operator new[](sizeof(unsigned long) * ulongs.size(), (std::align_val_t)(128));

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (unsigned int j = 0; j < ulongs.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			ulongsCopy[j] = ulongs[j];
			sorted[j] = j;									// page in the destination array into system memory
		}
		// Eliminate compiler ability to optimize paging-in of the input and output arrays
		// Paging-in source and destination arrays leads to a 50% speed-up on Linux, and 15% on Windows

		vector<unsigned long> sorted_reference(ulongs);
		sort(sorted_reference.begin(), sorted_reference.end());

		printf("ulongsCopy address = %p   sorted address = %p   value at a random location = %lu %lu\n", ulongsCopy, sorted, sorted[static_cast<unsigned>(rd()) % ulongs.size()], ulongsCopy[static_cast<unsigned>(rd()) % ulongs.size()]);
		const auto startTime = high_resolution_clock::now();
		//RadixSortLSDPowerOf2Radix_unsigned_TwoPhase(ulongsCopy, sorted, (unsigned long)ulongs.size());
		//sort_radix_in_place_adaptive(ulongsCopy, (unsigned long)ulongs.size(), 0.1);
		sort_radix_in_place_stable_adaptive(ulongsCopy, (unsigned long)ulongs.size(), 0.9);

		//RadixSortLSDPowerOf2Radix_unsigned_TwoPhase_DeRandomize(ulongsCopy, sorted, (unsigned long)ulongs.size());
		//RadixSortLSDPowerOf2RadixParallel_unsigned_TwoPhase(ulongsCopy, sorted, (unsigned long)ulongs.size());
		const auto endTime = high_resolution_clock::now();
		print_results("Radix Sort LSD", sorted, ulongs.size(), startTime, endTime);
		if (std::equal(sorted_reference.begin(), sorted_reference.end(), ulongsCopy))
			printf("Arrays are equal\n");
		else
		{
			printf("Arrays are not equal\n");
			exit(1);
		}
	}

	delete[] sorted;
	//delete[](operator new[](sizeof(intptr_t) * ulongs.size(), (std::align_val_t)(64)));
	delete [] ulongsCopy;
	//delete[](operator new[](sizeof(intptr_t) * ulongs.size(), (std::align_val_t)(64)));

	return 0;
}

int ParallelRadixSortLsdBenchmark(vector<unsigned long>& ulongs)
{
	random_device rd;

	// generate some random ulongs:
	//unsigned long* ulongsCopy = new unsigned long[ulongs.size()];
	unsigned long* ulongsCopy = static_cast<unsigned long*>(operator new[](sizeof(unsigned long) * ulongs.size(), (std::align_val_t)(128)));
	//unsigned long* ulongsCopy = new(std::align_val_t{ 128 }) unsigned long[ulongs.size()];
	//unsigned long* ulongsCopy  = (unsigned long*) operator new[](sizeof(unsigned long) * ulongs.size(), (std::align_val_t)(128));
	//unsigned long* tmp_working = new(std::align_val_t{ 128 }) unsigned long[ulongs.size()];
	//unsigned long* tmp_working = new unsigned long[ulongs.size()];
	unsigned long* tmp_working = static_cast<unsigned long*>(operator new[](sizeof(unsigned long) * ulongs.size(), (std::align_val_t)(128)));

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (unsigned int j = 0; j < ulongs.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			ulongsCopy[j] = ulongs[j];
			tmp_working[j] = j;								// page in the destination array into system memory
		}
		// Eliminate compiler ability to optimize paging-in of the input and output arrays
		// Paging-in source and destination arrays leads to a 50% speed-up on Linux, and 15% on Windows

		vector<unsigned long> sorted_reference(ulongs);
		sort(sorted_reference.begin(), sorted_reference.end());

		printf("ulongsCopy address = %p   sorted address = %p   value at a random location = %lu %lu\n", ulongsCopy, tmp_working, tmp_working[static_cast<unsigned>(rd()) % ulongs.size()], ulongsCopy[static_cast<unsigned>(rd()) % ulongs.size()]);
		const auto startTime = high_resolution_clock::now();
		//RadixSortLSDPowerOf2RadixScalar_unsigned_TwoPhase(ulongsCopy, tmp_working, (unsigned long)ulongs.size());
		//RadixSortLSDPowerOf2RadixParallel_unsigned_TwoPhase_DeRandomize(ulongsCopy, tmp_working, (unsigned long)ulongs.size());
		SortRadixPar(ulongsCopy, tmp_working, ulongs.size(), ulongs.size() / 6);
		//SortRadixPar(ulongsCopy, tmp_working, ulongs.size());
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Radix Sort LSD", tmp_working, ulongs.size(), startTime, endTime);
		if (std::equal(sorted_reference.begin(), sorted_reference.end(), ulongsCopy))
			printf("Arrays are equal\n");
		else
		{
			printf("Arrays are not equal\n");
			exit(1);
		}
	}

	//delete[] tmp_working;
	::operator delete[](tmp_working, std::align_val_t{ 128 });
	//::operator delete[](new(std::align_val_t{ 128 }) unsigned long[ulongs.size()], std::align_val_t{ 128 });
	//delete[](operator new[](sizeof(unsigned long) * ulongs.size(), (std::align_val_t)(128)));
	//delete[] ulongsCopy;
	::operator delete[](ulongsCopy, std::align_val_t{ 128 });
	//::operator delete[](new(std::align_val_t{ 128 }) unsigned long[ulongs.size()], std::align_val_t{ 128 });
	//::operator delete[](ulongsCopy, std::align_val_t{ 128 });
	//delete[](operator new[](sizeof(unsigned long) * ulongs.size(), (std::align_val_t)(128)));

	return 0;
}
