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

#include "RadixPartition.h"
//#include "RadixSelect.h"

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;
using std::random_device;
using std::sort;
using std::vector;

const int iterationCount = 5;

static void print_results(int iteration, const char* const tag, const unsigned* sorted, size_t sortedLength,
	high_resolution_clock::time_point startTime, high_resolution_clock::time_point endTime)
{
	printf("%d %s: Lowest: %u Highest: %u Time: %fms\n", iteration, tag,
		sorted[0], sorted[sortedLength - 1],
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

int RadixPartitionBenchmark(vector<unsigned>& uints)
{
	vector<unsigned> uintsCopy(uints);
	vector<unsigned> tmp_working(uints); // temporary working array/buffer
	size_t k = uints.size() - 21;   // valid values are 0 to uint.size() - 1

	for (int i = 0; i < iterationCount; ++i)
	{
		for (size_t j = 0; j < uints.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			uintsCopy[j] = uints[j];
			tmp_working[j] = (unsigned)j;									// page in the destination array into system memory
		}
		// Eliminate compiler ability to optimize paging-in of the input and output arrays
		// Paging-in source and destination arrays leads to a 50% speed-up on Linux, and 15% on Windows

		vector<unsigned> a_reference(uints);
		//sort(std::execution::par_unseq, a_reference.begin(), a_reference.end());
		const auto startTime_0 = high_resolution_clock::now();
		std::nth_element(a_reference.begin(), a_reference.end() - 21, a_reference.end());
		const auto endTime_0 = high_resolution_clock::now();
		printf("nth_element: Time: %fms\n", duration_cast<duration<double, milli>>(endTime_0 - startTime_0).count());

		//printf("uintsCopy address = %p   sorted address = %p   value at a random location = %lu %lu\n", uintsCopy, sorted, sorted[static_cast<unsigned>(rd()) % uints.size()], uintsCopy[static_cast<unsigned>(rd()) % uints.size()]);
		const auto startTime = high_resolution_clock::now();
		PartitionRadix(uintsCopy.data(), uints.size(), k);
		//unsigned selectedValue = SelectRadix(uintsCopy.data(), uints.size(), k);
		unsigned selectedValue = uintsCopy[k];
		const auto endTime = high_resolution_clock::now();
		print_results(i, "Radix Partition", uintsCopy.data(), uints.size(), startTime, endTime);
		if (selectedValue != a_reference[k])
		{
			printf("Selected value does not match reference value\n");
			printf("Difference: selected value = %x, reference value = %x\n", selectedValue, a_reference[k]);
			exit(1);
		}
	}
	return 0;
}
