// ParallelAlgorithms main application entry point

#include <iostream>
#include <random>
#include <ratio>
#include <vector>

using std::random_device;
using std::vector;

extern int ParallelStdCppExample(            vector<double>&   doubles);
extern int ParallelStdCppExample(            vector<unsigned>& uints, bool stable = false);
extern int ParallelStdCppExample(            vector<unsigned>& uints);
extern int RadixSortLsdBenchmark(            vector<unsigned>& uints);
extern int ParallelMergeSortBenchmark(       vector<double>&   doubles);
extern int ParallelMergeSortBenchmark(       vector<unsigned>& uints, const size_t& testSize);
extern int ParallelInPlaceMergeSortBenchmark(vector<unsigned>& uints);
extern int ParallelMergeSortBenchmark(       vector<unsigned>& uints);
extern int main_quicksort();
extern int ParallelMergeBenchmark();
extern int ParallelRadixSortLsdBenchmark(    vector<unsigned>& uints);
extern int RadixSortMsdBenchmark(            vector<unsigned>& uints);
extern void TestAverageOfTwoIntegers();
extern int CountingSortBenchmark(            vector<unsigned>& uints);
extern int SumBenchmark(                     vector<unsigned>& uints);
extern int SumBenchmarkChar(                 vector<unsigned>& uints);
extern int SumBenchmark64(                   vector<unsigned>& uints);
extern int TestMemoryAllocation();
extern int std_parallel_sort_leak_demo();
extern int bundling_small_work_items_benchmark(size_t, size_t, size_t);

int main()
{
	// Test configuration options
	bool UseStableStdSort = false;
	
	// Test cases for averaging of two integers
	//TestAverageOfTwoIntegers();

	// Benchmark QuickSort
	//main_quicksort();

#if 0
	// Demonstrate Parallel Merge

	const unsigned long A_NumElements = 8;
	const unsigned long B_NumElements = 6;
	unsigned long a_array[A_NumElements + B_NumElements] = { 2, 3, 3, 5, 15, 17, 20, 22, 0, 0, 6, 9, 16, 17 };	// first array has 8 elements, second array has 6
	const unsigned long C_NumElements = A_NumElements + B_NumElements;
	unsigned long c_array[C_NumElements];

	//merge_parallel_L5(a_array, 0, A_NumElements - 1, A_NumElements, C_NumElements - 1, c_array, 0);

	std::cout << std::endl << "merged array: ";
	for (unsigned long i = 0; i < C_NumElements; i++)
		std::cout << c_array[i] << " ";
	std::cout << std::endl;

	// Demonstrate Parallel Merge Sort

	const size_t NumElements = 8;
	//unsigned long unsorted_array[NumElements] = { 10, 3, 5, 2, 4, 11, 0, 3 };
	unsigned long   sorted_array[NumElements];

	//parallel_merge_sort_simplest(unsorted_array, 0, NumElements - 1, sorted_array);	// simplest, but slowest
	//ParallelAlgorithms::parallel_merge_sort_hybrid_rh_1(unsorted_array, 0, NumElements - 1, sorted_array);	// fastest

	std::cout << "sorted array: ";
	for (size_t i = 0; i < NumElements; i++)
	{
		std::cout << sorted_array[i] << " ";
		//std::cout << unsorted_array[i] << " ";
	}
	std::cout << std::endl << std::endl;
#endif

	// Provide the same input random array of doubles to all sorting algorithms
	const size_t testSize = 1'000'000'000;
	//random_device rd;
	std::mt19937_64 dist(1234);

	//const auto processor_count = std::thread::hardware_concurrency();
	//printf("Number of cores = %u \n", processor_count);

#if 0
	// generate some random doubles:
	printf("Testing with %zu random doubles...\n", testSize);
	vector<double> doubles(testSize);
	for (auto& d : doubles) {
		d = static_cast<double>(rd());
	}
	// Example of C++17 Standard C++ Parallel Sorting
	//ParallelStdCppExample(doubles);

	// Benchmark the above Parallel Merge Sort algorithm
	ParallelMergeSortBenchmark(doubles);
#endif
	// generate some random unsigned integers:
	printf("\nTesting with %zu random unsigned integers...\n\n", testSize);
	vector<unsigned> uints(testSize);
	for (auto& d : uints) {
		//d = static_cast<unsigned>(rd());
		d = static_cast<unsigned>(dist());   // way faster on Linux
	}
	// Example of C++17 Standard C++ Parallel Sorting
	//ParallelStdCppExample(uints, UseStableStdSort);

	bundling_small_work_items_benchmark(20, 10000, 1000);

//	std_parallel_sort_leak_demo();
//	return 0;

//	RadixSortMsdBenchmark(uints);

	//CountingSortBenchmark(uints);		// sorts uchar's and not ulongs

	//SumBenchmarkChar(uints);
	SumBenchmark(    uints);
	//SumBenchmark64(  uints);

	return 0;

	// Benchmark the above Parallel Merge Sort algorithm
	ParallelMergeSortBenchmark(uints, testSize);

	ParallelInPlaceMergeSortBenchmark(uints);

	ParallelRadixSortLsdBenchmark(uints);

	ParallelMergeBenchmark();

	RadixSortLsdBenchmark(uints);

	// generate some nearly pre-sorted unsigned integers:
	printf("\nTesting with %zu nearly pre-sorted unsigned integers...\n\n", testSize);
	//vector<unsigned> uints(testSize);
	for (size_t i = 0; i < uints.size(); i++) {
		if ((i % 100) == 0)
		{
			//uints[i] = static_cast<unsigned>(rd());
			uints[i] = static_cast<unsigned>(dist());   // way faster on Linux
		}
		else
			uints[i] = static_cast<unsigned>(i);
	}
	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(uints, UseStableStdSort);

	RadixSortMsdBenchmark(uints);

	//CountingSortBenchmark(uints);		// sorts uchar's and not ulongs

	SumBenchmarkChar(uints);

	SumBenchmark(uints);

	// Benchmark the above Parallel Merge Sort algorithm
	ParallelMergeSortBenchmark(uints, testSize);

	ParallelInPlaceMergeSortBenchmark(uints);

	ParallelRadixSortLsdBenchmark(uints);

	RadixSortLsdBenchmark(uints);

	printf("\nTesting with %zu constant unsigned integers...\n\n", testSize);
	for (size_t i = 0; i < uints.size(); i++) {
		uints[i] = 10;
	}

	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(uints, UseStableStdSort);

	RadixSortMsdBenchmark(uints);

	//CountingSortBenchmark(uints);		// sorts uchar's and not ulongs

	SumBenchmarkChar(uints);

	SumBenchmark(uints);

	// Benchmark the above Parallel Merge Sort algorithm
	ParallelMergeSortBenchmark(uints, testSize);

	ParallelInPlaceMergeSortBenchmark(uints);

	ParallelRadixSortLsdBenchmark(uints);

	RadixSortLsdBenchmark(uints);

#if 0
	// generate some random unsigned longs:
	printf("\nTesting with %zu random unsigned longs longs...\n\n", testSize);
	vector<unsigned long long> ulonglongs(testSize);
	for (auto& d : ulonglongs) {
		//d = static_cast<unsigned long long>(rd());
		d = static_cast<unsigned long long>(dist());   // way faster on Linux
	}
	printf("Finished initializing unsigned long long array\n");

#if 1
	//RadixSortMsdBenchmark(ulongs);

	//CountingSortBenchmark(ulongs);	// sorts uchar's and not ulongs

	//SumBenchmarkChar(ulongs);
	//SumBenchmark(ulongs);

	// Example of C++17 Standard C++ Parallel Sorting
	//ParallelStdCppExample(ulongs, UseStableStdSort);

	// Benchmark the above Parallel Merge Sort algorithm
	//ParallelMergeSortBenchmark(ulongs, testSize);

#endif
	// Benchmark Parallel InPlace Merge Sort algorithm
	//ParallelInPlaceMergeSortBenchmark(ulongs);

	// Benchmark Radix Sort LSD algorithm
	//RadixSortLsdBenchmark(ulongs);

	// Benchmark Radix Sort LSD algorithm
	//ParallelRadixSortLsdBenchmark(ulongs);

	printf("\nTesting with %zu nearly pre-sorted unsigned long longs...\n\n", testSize);
	for (size_t i = 0; i < ulonglongs.size(); i++) {
		if ((i % 100) == 0)
		{
			//ulongs[i] = static_cast<unsigned long>(rd());
			ulonglongs[i] = static_cast<unsigned long long>(dist());   // way faster on Linux
		}
		else
			ulonglongs[i] = static_cast<unsigned long long>(i);
	}

	//CountingSortBenchmark(ulongs);	// sorts uchar's and not ulongs

	//SumBenchmarkChar(ulongs);
	//SumBenchmark(ulongs);

	//RadixSortMsdBenchmark(ulongs);

	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(ulongs, UseStableStdSort);

	// Benchmark the above Parallel Merge Sort algorithm
	//ParallelMergeSortBenchmark(ulongs, testSize);

	// Benchmark Parallel InPlace Merge Sort algorithm
	//ParallelInPlaceMergeSortBenchmark(ulongs);

	// Benchmark the above Parallel Merge Sort algorithm
	//RadixSortLsdBenchmark(ulongs);

	// Benchmark Radix Sort LSD algorithm
	//ParallelRadixSortLsdBenchmark(ulongs);

	printf("\nTesting with %zu constant unsigned long longs...\n\n", testSize);
	for (size_t i = 0; i < ulonglongs.size(); i++) {
		ulonglongs[i] = 10;
	}

	//CountingSortBenchmark(ulongs);	// sorts uchar's and not ulongs

	//SumBenchmarkChar(ulongs);
	//SumBenchmark(ulongs);

	//RadixSortMsdBenchmark(ulongs);

	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(ulongs, UseStableStdSort);

	// Benchmark the above Parallel Merge Sort algorithm
	//ParallelMergeSortBenchmark(ulongs, testSize);

	// Benchmark Parallel InPlace Merge Sort algorithm
	//ParallelInPlaceMergeSortBenchmark(ulongs);

	// Benchmark the above Parallel Merge Sort algorithm
	//RadixSortLsdBenchmark(ulongs);

	// Benchmark Radix Sort LSD algorithm
	//ParallelRadixSortLsdBenchmark(ulongs);
#endif

	return 0;
}
