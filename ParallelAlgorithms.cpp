// ParallelAlgorithms.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <random>
#include <ratio>
#include <vector>
#include <execution>
#include "ParallelMergeSort.h"

using std::random_device;
using std::vector;

extern int ParallelStdCppExample(vector<double>& doubles);
extern int ParallelMergeSortBenchmark(vector<double>& doubles);

int main()
{
	// Demonstrate Paralle Merge

	const unsigned long A_NumElements = 8;
	const unsigned long B_NumElements = 6;
	unsigned long a_array[A_NumElements + B_NumElements] = { 2, 3, 3, 5, 15, 17, 20, 22, 0, 0, 6, 9, 16, 17 };	// first array has 8 elements, second array has 6
	const unsigned long C_NumElements = A_NumElements + B_NumElements;
	unsigned long c_array[C_NumElements];

	merge_parallel_L5(a_array, 0, A_NumElements - 1, A_NumElements, C_NumElements - 1, c_array, 0);

	std::cout << "merged array: ";
	for (unsigned long i = 0; i < C_NumElements; i++)
		std::cout << c_array[i] << " ";
	std::cout << std::endl;

	// Demonstrate Parallel Merge Sort

	const unsigned long NumElements = 8;
	unsigned long unsorted_array[NumElements] = { 10, 3, 5, 2, 4, 11, 0, 3 };
	unsigned long   sorted_array[NumElements];

	//parallel_merge_sort_simplest(unsorted_array, 0, NumElements - 1, sorted_array);	// simplest, but slowest
	parallel_merge_sort_hybrid_rh_1(unsorted_array, 0, NumElements - 1, sorted_array);	// fastest

	std::cout << "sorted array: ";
	for (unsigned long i = 0; i < NumElements; i++)
		std::cout << sorted_array[i] << " ";
	std::cout << std::endl << std::endl;

	// Provide the same input random array of doubles to all sorting algorithms
	const size_t testSize = 1'000'000;
	random_device rd;

	// generate some random doubles:
	printf("Testing with %zu doubles...\n", testSize);
	vector<double> doubles(testSize);
	for (auto& d : doubles) {
		d = static_cast<double>(rd());
	}
	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(doubles);

	// Benchmark the above Parallel Merge Sort algorithm
	ParallelMergeSortBenchmark(doubles);

    return 0;
}
