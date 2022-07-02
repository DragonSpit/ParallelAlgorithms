// ParallelAlgorithms.cpp : Defines the entry point for the console application.
// TODO: TBB includes Parallel STL algorithm implementations, including sort. To use them pstl::execution execution policy needs to be use instead of std::execution, to
//       select TBB PSTL algorithm. It would be interesting to compare performance of standard and TBB parallel sort implementations, and update my blog to include these results.

#include <iostream>
#include <random>
#include <ratio>
#include <vector>
#include "ParallelMergeSort.h"

using std::random_device;
using std::vector;

extern int ParallelStdCppExample(vector<double>&             doubles);
extern int ParallelStdCppExample(vector<unsigned long>&      ulongs, bool stable = false);
extern int ParallelTbbCppExample(vector<unsigned long>& ulongs);
extern int ParallelStdCppExample(vector<unsigned>&           uints);
extern int RadixSortLsdBenchmark(vector<unsigned long>&      ulongs);
extern int ParallelMergeSortBenchmark(vector<double>&        doubles);
extern int ParallelMergeSortBenchmark(vector<unsigned long>& ulongs);
extern int ParallelInPlaceMergeSortBenchmark(vector<unsigned long>& ulongs);
extern int ParallelMergeSortBenchmark(vector<unsigned>&      uints);
extern int main_quicksort();
extern int ParallelMergeBenchmark();
extern int ParallelRadixSortLsdBenchmark(vector<unsigned long>& ulongs);
extern int RadixSortMsdBenchmark(vector<unsigned long>& ulongs);
extern void TestAverageOfTwoIntegers();

int main()
{
	// Test configuration options
	bool UseStableStdSort = true;
	// Test cases for averaging of two integers
	TestAverageOfTwoIntegers();

	// Benchmark QuickSort
	main_quicksort();

	// Demonstrate Parallel Merge

	const unsigned long A_NumElements = 8;
	const unsigned long B_NumElements = 6;
	unsigned long a_array[A_NumElements + B_NumElements] = { 2, 3, 3, 5, 15, 17, 20, 22, 0, 0, 6, 9, 16, 17 };	// first array has 8 elements, second array has 6
	const unsigned long C_NumElements = A_NumElements + B_NumElements;
	unsigned long c_array[C_NumElements];

	merge_parallel_L5(a_array, 0, A_NumElements - 1, A_NumElements, C_NumElements - 1, c_array, 0);

	std::cout << std::endl << "merged array: ";
	for (unsigned long i = 0; i < C_NumElements; i++)
		std::cout << c_array[i] << " ";
	std::cout << std::endl;

	// Demonstrate Parallel Merge Sort

	const unsigned long NumElements = 8;
	unsigned long unsorted_array[NumElements] = { 10, 3, 5, 2, 4, 11, 0, 3 };
	unsigned long   sorted_array[NumElements];

	//parallel_merge_sort_simplest(unsorted_array, 0, NumElements - 1, sorted_array);	// simplest, but slowest
	ParallelAlgorithms::parallel_merge_sort_hybrid_rh_1(unsorted_array, 0, NumElements - 1, sorted_array);	// fastest

	std::cout << "sorted array: ";
	for (unsigned long i = 0; i < NumElements; i++)
	{
		std::cout << sorted_array[i] << " ";
		//std::cout << unsorted_array[i] << " ";
	}
	std::cout << std::endl << std::endl;

	// Provide the same input random array of doubles to all sorting algorithms
	const size_t testSize = 10'000'000;
	random_device rd;

	const auto processor_count = std::thread::hardware_concurrency();
	printf("Number of cores = %u \n", processor_count);

#if 0
	// generate some random doubles:
	printf("Testing with %zu rendom doubles...\n", testSize);
	vector<double> doubles(testSize);
	for (auto& d : doubles) {
		d = static_cast<double>(rd());
	}
	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(doubles);

	// Benchmark the above Parallel Merge Sort algorithm
	ParallelMergeSortBenchmark(doubles);
#endif

	// generate some random unsigned longs:
	printf("\nTesting with %zu random unsigned longs...\n\n", testSize);
	vector<unsigned long> ulongs(testSize);
	for (auto& d : ulongs) {
		d = static_cast<unsigned long>(rd());
	}
	printf("Finished initializing unsigned long array\n");

#if 1
	RadixSortMsdBenchmark(ulongs);

	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(ulongs, UseStableStdSort);

	// Example of C++17 Standard C++ Parallel Sorting
	//ParallelTbbCppExample(ulongs);

	// Benchmark the above Parallel Merge Sort algorithm
	ParallelMergeSortBenchmark(ulongs);

	// Benchmark Parallel InPlace Merge Sort algorithm
	ParallelInPlaceMergeSortBenchmark(ulongs);
#endif
	// Benchmark Radix Sort LSD algorithm
	RadixSortLsdBenchmark(ulongs);

	// Benchmark Radix Sort LSD algorithm
	ParallelRadixSortLsdBenchmark(ulongs);

	printf("\nTesting with %zu nearly pre-sorted unsigned longs...\n\n", testSize);
	for (size_t i = 0; i < ulongs.size(); i++) {
		if ((i % 100) == 0)
			ulongs[i] = static_cast<unsigned long>(rd());
		else
			ulongs[i] = static_cast<unsigned long>(i);
	}

	RadixSortMsdBenchmark(ulongs);

	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(ulongs, UseStableStdSort);

	// Benchmark the above Parallel Merge Sort algorithm
	ParallelMergeSortBenchmark(ulongs);

	// Benchmark Parallel InPlace Merge Sort algorithm
	ParallelInPlaceMergeSortBenchmark(ulongs);

	// Benchmark the above Parallel Merge Sort algorithm
	RadixSortLsdBenchmark(ulongs);

	// Benchmark Radix Sort LSD algorithm
	ParallelRadixSortLsdBenchmark(ulongs);

	printf("\nTesting with %zu constant unsigned longs...\n\n", testSize);
	for (size_t i = 0; i < ulongs.size(); i++) {
		ulongs[i] = 10;
	}

	RadixSortMsdBenchmark(ulongs);

	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(ulongs, UseStableStdSort);

	// Benchmark the above Parallel Merge Sort algorithm
	ParallelMergeSortBenchmark(ulongs);

	// Benchmark Parallel InPlace Merge Sort algorithm
	ParallelInPlaceMergeSortBenchmark(ulongs);

	// Benchmark the above Parallel Merge Sort algorithm
	RadixSortLsdBenchmark(ulongs);

	// Benchmark Radix Sort LSD algorithm
	ParallelRadixSortLsdBenchmark(ulongs);

	// generate some random unsigned integers:
	printf("\nTesting with %zu random unsigned integers...\n\n", testSize);
	vector<unsigned> uints(testSize);
	for (auto& d : uints) {
		d = static_cast<unsigned>(rd());
	}
	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(uints);

	// Benchmark the above Parallel Merge Sort algorithm
	ParallelMergeSortBenchmark(uints);

	// generate some nearly pre-sorted unsigned integers:
	printf("\nTesting with %zu nearly pre-sorted unsigned integers...\n\n", testSize);
	//vector<unsigned> uints(testSize);
	for (size_t i = 0; i < uints.size(); i++) {
		if ((i % 100) == 0)
			uints[i] = static_cast<unsigned>(rd());
		else
			uints[i] = static_cast<unsigned>(i);
	}
	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(uints);

	// Benchmark the above Parallel Merge Sort algorithm
	ParallelMergeSortBenchmark(uints);

	ParallelMergeBenchmark();

	return 0;
}
