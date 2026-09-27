#include <stddef.h>
#include <stdio.h>
#include <array>
#include <functional>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <ratio>
#include <vector>
#include <execution>
#include <set>
#include <unordered_set>

#include "RadixSelect.h"

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;
using std::random_device;
using std::sort;
using std::vector;

const int iterationCount = 5;

int DeDuplicateBenchmark(vector<unsigned>& uints)
{
	for (int i = 0; i < iterationCount; ++i)
	{
		vector<unsigned> uintsCopy(uints);
		vector<unsigned> tmp_working(uints); // temporary working array/buffer
		for (size_t j = 0; j < uints.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			uintsCopy[j] = uints[j];
			tmp_working[j] = (unsigned)j;									// page in the destination array into system memory
		}

		vector<unsigned> a_reference(uints);
#if 0
		// sort-unique method 1
		const auto startTime_0 = high_resolution_clock::now();
		sort(a_reference.begin(), a_reference.end());
		auto last_ref = unique(a_reference.begin(), a_reference.end());	
		const auto endTime_0 = high_resolution_clock::now();
		auto uniqueCount = std::distance(a_reference.begin(), last_ref);
		printf("sort-unique method: Time: %fms with %lld unique elements\n", duration_cast<duration<double, milli>>(endTime_0 - startTime_0).count(), uniqueCount);
		a_reference.erase(last_ref, a_reference.end());
#endif
#if 0
		// set method 2
		const auto startTime_0 = high_resolution_clock::now();
		std::set<unsigned> s(a_reference.begin(), a_reference.end());	// Construct a set (removes duplicates and sorts)
		a_reference.assign(s.begin(), s.end());							// Assign back to the vector
		const auto endTime_0 = high_resolution_clock::now();
		printf("set method: Time: %fms with %lld unique elements\n", duration_cast<duration<double, milli>>(endTime_0 - startTime_0).count(), a_reference.size());
#endif
#if 1
		// unordered_set method 3
		const auto startTime_0 = high_resolution_clock::now();
		std::unordered_set<int> seen;
		// Remove elements if they are already in the 'seen' set
		auto last_ref = std::remove_if(a_reference.begin(), a_reference.end(), [&seen](int x) {
			return !seen.insert(x).second; // .second is false if element already existed
			});
		const auto endTime_0 = high_resolution_clock::now();
		a_reference.erase(last_ref, a_reference.end());
		sort(a_reference.begin(), a_reference.end()); // Sort the unique elements for later comparison
		printf("unordered_set method: Time: %fms with %lld unique elements\n", duration_cast<duration<double, milli>>(endTime_0 - startTime_0).count(), a_reference.size());
#endif

		//printf("uintsCopy address = %p   sorted address = %p   value at a random location = %lu %lu\n", uintsCopy, sorted, sorted[static_cast<unsigned>(rd()) % uints.size()], uintsCopy[static_cast<unsigned>(rd()) % uints.size()]);
		const auto startTime = high_resolution_clock::now();
		//hybrid_inplace_msd_radix_sort_bins(uintsCopy.data(), (unsigned long)uintsCopy.size());				// In-place MSD Radix Sort
		RadixSortLSDPowerOf2Radix_unsigned_TwoPhase(uintsCopy.data(), tmp_working.data(), uints.size());	// Not-in-place LSD Radix Sort
		auto last = unique(uintsCopy.begin(), uintsCopy.end());
		const auto endTime = high_resolution_clock::now();
		auto uniqueCountGolden = std::distance(uintsCopy.begin(), last);
		printf("Radix sort-unique: Time: %fms with %lld unique elements\n", duration_cast<duration<double, milli>>(endTime - startTime).count(), uniqueCountGolden);
		uintsCopy.erase(last, uintsCopy.end());

		if (!std::equal(a_reference.begin(), a_reference.end(), uintsCopy.begin()))
		{
			printf("Arrays are not equal\n");
			exit(1);
		}
	}
	return 0;
}
