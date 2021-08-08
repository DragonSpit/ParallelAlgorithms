//#include <oneapi/dpl/execution>
//#include <oneapi/dpl/algorithm>

#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <ratio>
#include <vector>
#include <execution>

#include "ParallelMergeSort.h"
#include "SortParallel.h"

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
		ParallelAlgorithms::parallel_merge_sort_hybrid_rh_1(doublesCopy, 0, doubles.size() - 1, sorted);	// ParallelMergeSort modifies the source array
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
	printf("\nBenchmarking Parallel Merge Sort Hybrid with %zu unsigned longs (each of %lu bytes)...\n", ulongs.size(), (unsigned long)sizeof(unsigned long));
	unsigned long* ulongsCopy  = new unsigned long[ulongs.size()];
	unsigned long* ulongsCopy2 = new unsigned long[ulongs.size()];
	unsigned long* sorted      = new unsigned long[ulongs.size()];
	vector<unsigned long> ulongsCopyVec(ulongs);
	vector<unsigned long> ulongsCopyVec2(ulongs);

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (unsigned int j = 0; j < ulongs.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			ulongsCopy[ j] = ulongs[j];
			ulongsCopy2[j] = ulongs[j];
			sorted[j] = j;									// page in the destination array into system memory
		}
		const auto startTime = high_resolution_clock::now();
		// Example of usages, which trade off ease of use and performance
		//ParallelAlgorithms::sort_par(ulongsCopy, ulongs.size());										//     in-place adaptive interface
		//ParallelAlgorithms::sort_par(ulongsCopy, 0, ulongs.size());									//     in-place adaptive interface
		//ParallelAlgorithms::sort_par(ulongsCopy, ulongs.size(), sorted, ulongs.size(), false);		//     in-place interface
		//ParallelAlgorithms::sort_par(ulongsCopy, ulongs.size(), sorted, ulongs.size(), true);			// not in-place interface
		ParallelAlgorithms::sort_par(ulongsCopy, 0, ulongs.size(), sorted, ulongs.size(), false);		//     in-place interface
		//ParallelAlgorithms::sort_par(ulongsCopyVec);													//     in-place adaptive interface (vector)
		//sort(ulongsCopyVec.begin(), ulongsCopyVec.end());											//     in-place adaptive interface (vector)

		const auto endTime = high_resolution_clock::now();
		//sort(std::execution::par_unseq, ulongsCopy2, ulongsCopy2 + ulongs.size());
		sort(std::execution::par_unseq, ulongsCopyVec2.begin(), ulongsCopyVec2.end());
		print_results("Parallel Merge Sort", sorted, ulongs.size(), startTime, endTime);
		//if (std::equal(sorted, sorted + ulongs.size(), ulongsCopy2))
		//if (std::equal(ulongsCopy, ulongsCopy + ulongs.size(), ulongsCopy2))
		if (std::equal(ulongsCopyVec.begin(), ulongsCopyVec.end(), ulongsCopyVec2.begin()))
			std::cout << "Arrays are equal ";
		else
			std::cout << "Arrays are not equal ";
	}

	delete[] sorted;
	delete[] ulongsCopy2;
	delete[] ulongsCopy;

	return 0;
}

int ParallelInPlaceMergeSortBenchmark(vector<unsigned long>& ulongs)
{
	random_device rd;

	// generate some random uints:
	printf("\nBenchmarking InPlace Parallel Merge Sort Hybrid with %zu unsigned longs (each of %lu bytes)...\n", ulongs.size(), (unsigned long)sizeof(unsigned long));
	unsigned long* ulongsCopy  = new unsigned long[ulongs.size()];
	unsigned long* ulongsCopy2 = new unsigned long[ulongs.size()];
	unsigned long* sorted      = new unsigned long[ulongs.size()];

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (unsigned int j = 0; j < ulongs.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			ulongsCopy[j]  = ulongs[j];
			ulongsCopy2[j] = ulongs[j];
			sorted[j] = j;									// page in the destination array into system memory
		}
		ParallelAlgorithms::parallel_merge_sort_hybrid_rh_1(ulongsCopy, 0, (int)(ulongs.size() - 1), sorted);	// ParallelMergeSort modifies the source array
		//std::cout << "Before parallel inplace merge sort" << std::endl;
		const auto startTime = high_resolution_clock::now();
		//parallel_inplace_merge_sort_hybrid_inner(ulongsCopy2, 0, (int)(ulongs.size() - 1));
		//ParallelAlgorithms::parallel_inplace_merge_sort_hybrid(ulongsCopy2, 0, (int)(ulongs.size() - 1));
		ParallelAlgorithms::parallel_inplace_merge_sort_hybrid(ulongsCopy2, 0, ulongs.size() - 1, ulongs.size() / 48);
		//inplace_merge_sort_hybrid(ulongsCopy2, 0, (int)(ulongs.size() - 1));
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel InPlace Merge Sort", ulongsCopy, ulongs.size(), startTime, endTime);
		if (std::equal(sorted, sorted + ulongs.size(), ulongsCopy2))
			std::cout << "Arrays are equal ";
		else
			std::cout << "Arrays are not equal ";
	}

	delete[] sorted;
	delete[] ulongsCopy2;
	delete[] ulongsCopy;

	return 0;
}

int ParallelMergeSortBenchmark(vector<unsigned>& uints)
{
	random_device rd;

	// generate some random uints:
	printf("\nBenchmarking Parallel Merge Sort Hybrid with %zu unsigned integers (each of %lu bytes)...\n", uints.size(), (unsigned long)sizeof(unsigned));
	unsigned* uintsCopy = new unsigned[uints.size()];
	unsigned* sorted    = new unsigned[uints.size()];

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (unsigned int j = 0; j < uints.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			uintsCopy[j] = uints[j];
			sorted[j] = j;									// page in the destination array into system memory
		}
		const auto startTime = high_resolution_clock::now();
		//parallel_merge_sort_hybrid_rh_1(uintsCopy, 0, (int)(uints.size() - 1), sorted);	// ParallelMergeSort modifies the source array
		ParallelAlgorithms::parallel_merge_sort_hybrid(uintsCopy, 0, (int)(uints.size() - 1), sorted);	// ParallelMergeSort modifies the source array
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Merge Sort", sorted, uints.size(), startTime, endTime);
	}

	delete[] sorted;
	delete[] uintsCopy;

	return 0;
}

int ParallelMergeBenchmark()
{
	const size_t testSize = 10'000'000;
	random_device rd;

	// generate some random uints:
	vector<unsigned> uints_0(testSize);
	for (auto& d : uints_0)
		d = static_cast<unsigned>(rd());

	printf("\nBenchmarking Parallel Merge with %zu unsigned integers (each of %lu bytes)...\n", uints_0.size(), (unsigned long)sizeof(unsigned));

#if 1
	sort(std::execution::par_unseq, uints_0.begin(), uints_0.begin() + testSize/2);
	sort(std::execution::par_unseq, uints_0.begin() + testSize/2, uints_0.end());
#else
	sort(oneapi::dpl::execution::par_unseq, uints_0.begin(), uints_0.begin() + testSize / 2);
	sort(oneapi::dpl::execution::par_unseq, uints_0.begin() + testSize / 2, uints_0.end());
#endif

	// time how long it takes to merge them them:
	for (int i = 0; i < iterationCount; ++i)
	{
		vector<unsigned> uints_work(uints_0);		// copy the original into a working vector, since it's an in-place merge
		const auto startTime = high_resolution_clock::now();
		std::inplace_merge(std::execution::par_unseq, uints_work.begin(), uints_work.begin() + testSize / 2, uints_work.end());
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Merge", uints_work.data(), uints_work.size(), startTime, endTime);
	}
	// time how long it takes to merge them them:
	for (int i = 0; i < iterationCount; ++i)
	{
		vector<unsigned> uints_work(uints_0);		// copy the original into a working vector, since it's an in-place merge
		const auto startTime = high_resolution_clock::now();
		std::inplace_merge(uints_work.begin(), uints_work.begin() + testSize / 2, uints_work.end());
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Merge", uints_work.data(), uints_work.size(), startTime, endTime);
	}

	return 0;
}
