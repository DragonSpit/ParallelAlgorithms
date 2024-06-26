#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
//	#define DPL_ALGORITHMS          // Includes Intel's OneAPI parallel algorithm implementations
	#define MICROSOFT_ALGORITHMS    // Excludes single-core SIMD implementations, which Microsoft does not support
#endif

#ifdef DPL_ALGORITHMS
	// oneDPL headers should be included before standard headers
	#include <oneapi/dpl/algorithm>
	#include <oneapi/dpl/execution>
	#include <oneapi/dpl/iterator>
#else
	#include <iostream>
	#include <random>
	#include <ratio>
	#include <vector>
	#include <algorithm>
	#include <execution>
	#include <iterator>
	#include <chrono>
#endif

//#include <oneapi/dpl/algorithm>
//#define __TBB_PREVIEW_TASK_ARENA_CONSTRAINTS_EXTENSION_PRESENT 1
//#include <oneapi/tbb/task_arena.h>

#include "SumParallel.h"

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;
using std::random_device;
using std::sort;
using std::vector;

const int iterationCount = 20;

//extern void print_results(const char* const tag, const unsigned long long sum, size_t sum_array_length,
//	high_resolution_clock::time_point startTime, high_resolution_clock::time_point endTime);

void print_results(const char* const tag, const unsigned long long sum, size_t sum_array_length,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime)
{
	printf("%s: Sum: %llu   Array Length: %zu    Time: %fms   Throughput: %llu millions/second\n", tag, sum, sum_array_length,
		duration_cast<duration<double, milli>>(endTime - startTime).count(), (unsigned long long)(sum_array_length / duration_cast<duration<double, milli>>(endTime - startTime).count() * 1000.0 / 1000000.0));
}
void print_results(const char* const tag, const unsigned long long sum, size_t sum_array_length,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime,
	double thruput_average, double thruput_std_dev
)
{
	printf("%s: Sum: %llu   Array Length: %zu    Time: %fms  Throughput Average: %.lf million   Standard Deviation: %.lf\n", tag, sum, sum_array_length,
		duration_cast<duration<double, milli>>(endTime - startTime).count(), thruput_average, thruput_std_dev);
}

// From: https://stackoverflow.com/questions/7616511/calculate-mean-and-standard-deviation-from-a-vector-of-samples-in-c-using-boos
double std_deviation(vector<double>& v)
{
	double sum = std::accumulate(v.begin(), v.end(), 0.0);
	double mean = sum / v.size();

	std::vector<double> diff(v.size());
	//std::transform(v.begin(), v.end(), diff.begin(), std::bind2nd(std::minus<double>(), mean));
	std::transform(v.begin(), v.end(), diff.begin(), [mean](double x) { return x - mean; });
	double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	double stdev = std::sqrt(sq_sum / v.size());
	return(stdev);
}

int SumBenchmarkChar(vector<unsigned>& uints)
{
	vector<unsigned char> u8Copy(uints.size());
	vector<unsigned char> u8Array(uints.size());

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (size_t j = 0; j < uints.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			u8Array[j] = (unsigned char)uints[j];
			u8Copy[ j] = (unsigned char)uints[j];
		}
		// Eliminate compiler ability to optimize paging-in of the input and output arrays
		// Paging-in source and destination arrays leads to a 50% speed-up on Linux, and 15% on Windows

		const auto startTimeRef = high_resolution_clock::now();
		unsigned long long sum_ref = 0;
		//for (size_t i = 0; i < uints.size(); i++)
		//	sum_ref += u8Copy[i];
		sum_ref = std::accumulate(u8Copy.begin(), u8Copy.end(), 0ULL);
		const auto endTimeRef = high_resolution_clock::now();
		print_results("std::accumulate", sum_ref, u8Copy.size(), startTimeRef, endTimeRef);

		unsigned long long sum = 0;
		//for (size_t k = 0; k < 100; k++)
		//{
			const auto startTime = high_resolution_clock::now();
			//long long sum = ParallelAlgorithms::SumParallel(u8Array, 0, uints.size());
			//sum = ParallelAlgorithms::SumParallel(u8Array, 0, uints.size(), uints.size() / 24);	// Running on 24-core is fastest, however with 2.7X run-to-run variation
			sum = ParallelAlgorithms::SumParallel(u8Array.data(), 0, uints.size());
			const auto endTime = high_resolution_clock::now();
			print_results("Parallel Sum of uchars", sum, uints.size(), startTime, endTime);
		//}
		if (sum == sum_ref)
			printf("Sums are equal\n");
		else
		{
			printf("Sums are not equal\n");
			exit(1);
		}
	}
	return 0;
}

int SumBenchmark(vector<unsigned>& uints)
{
	vector<unsigned> u32Copy( uints.size());
	vector<unsigned> u32Array(uints.size());

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (size_t j = 0; j < uints.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			u32Array[j] = (unsigned)uints[j];
			u32Copy[ j] = (unsigned)uints[j];
		}
		// Eliminate compiler ability to optimize paging-in of the input and output arrays
		// Paging-in source and destination arrays leads to a 50% speed-up on Linux, and 15% on Windows

		const auto startTimeRef = high_resolution_clock::now();
		long long sum_ref = 0;
		//for (size_t i = 0; i < uints.size(); i++)
		//	sum_ref += u8Copy[i];
		sum_ref = std::accumulate(u32Copy.begin(), u32Copy.end(), 0LL);
		const auto endTimeRef = high_resolution_clock::now();
		print_results("std::accumulate", sum_ref, u32Copy.size(), startTimeRef, endTimeRef);

		long long sum = 0;
		//for (size_t k = 0; k < 100; k++)
		//{
		const auto startTime = high_resolution_clock::now();
		//long long sum = ParallelAlgorithms::SumParallel(u8Array, 0, uints.size());
		//sum = ParallelAlgorithms::SumParallel(u8Array, 0, uints.size(), uints.size() / 24);	// Running on 24-core is fastest, however with 2.7X run-to-run variation
		//sum = ParallelAlgorithms::SumParallel(u32Array.data(), 0, uints.size());
		sum = ParallelAlgorithms::SumParallelNonRecursive(u32Array.data(), 0, uints.size());
		//sum = ParallelAlgorithms::SumParallelNonRecursiveBuffered(u32Array.data(), 0, uints.size());
		//sum = ParallelAlgorithms::SumParallelNonRecursiveBufferedLocally(u32Array.data(), 0, uints.size());
		//sum = ParallelAlgorithms::SumParallelNonRecursiveNoHyperthreading(u32Array.data(), 0, uints.size());
		//sum = ParallelAlgorithms::SumParallelNonRecursiveBufferedLocallyNoHyperthreading(u32Array.data(), 0, uints.size());
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Sum of unsigned", sum, uints.size(), startTime, endTime);
		//}
		if (sum == sum_ref)
			printf("Sums are equal\n");
		else
		{
			printf("Sums are not equal\n");
			exit(1);
		}
	}
	return 0;
}

int SumBenchmark64(vector<unsigned>& uints)
{
	vector<unsigned long long> u64Copy( uints.size());
	vector<unsigned long long> u64Array(uints.size());
	size_t num_times = 10;
	double thruput_sum;
	std::vector<double> thruputs(num_times);

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (size_t j = 0; j < uints.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			u64Array[j] = (unsigned long long)uints[j];
			u64Copy[ j] = (unsigned long long)uints[j];
		}
		// Eliminate compiler ability to optimize paging-in of the input and output arrays
		// Paging-in source and destination arrays leads to a 50% speed-up on Linux, and 15% on Windows

		const auto startTimeRef = high_resolution_clock::now();
		unsigned long long sum_ref = 0;
		//for (size_t i = 0; i < uints.size(); i++)
		//	sum_ref += u64Copy[i];
		//sum_ref = std::accumulate(u64Copy.begin(), u64Copy.end(), 0ULL);
		sum_ref = std::accumulate(u64Copy.begin(), u64Copy.end(), 0ULL);
		//std::fill(oneapi::dpl::execution::par_unseq, u64Copy.begin(), u64Copy.end(), 42);
		//std::fill(u64Copy.begin(), u64Copy.end(), 42);
		const auto endTimeRef = high_resolution_clock::now();
		print_results("std::accumulate", sum_ref, u64Copy.size(), startTimeRef, endTimeRef);
		//unsigned long long sum_array[1000] = { 0 };

		auto startTime = high_resolution_clock::now();
		auto endTime   = high_resolution_clock::now();
		unsigned long long sum = 0;
		thruput_sum = 0.0;
		for (size_t j = 0; j < num_times; ++j)
		{
			startTime = high_resolution_clock::now();
			sum = 0;

			//unsigned long long sum = ParallelAlgorithms::SumParallel(u64Array, 0, uints.size());	// Running on 24-core is fastest, however with 2.7X run-to-run variation
			//unsigned long long sum = ParallelAlgorithms::SumParallel(u64Array, 0, uints.size(), uints.size() / 24);	// Running on 24-core is fastest, however with 2.7X run-to-run variation
			//unsigned long long sum = ParallelAlgorithms::SumParallelNonRecursive(u64Array, 0, uints.size());
			//unsigned long long sum = ParallelAlgorithms::SumParallelNonRecursive(u64Array, 0, uints.size(), uints.size() / 8);
			//sum = ParallelAlgorithms::SumParallelNonRecursiveNoHyperthreading(u64Array, 0, uints.size(), uints.size() / 14);
			//sum = ParallelAlgorithms::SumNonRecursive(u64Array, 0, uints.size(), uints.size() / 2);
			//sum = ParallelAlgorithms::SumParallelNonRecursive(u64Array, 0, uints.size(), uints.size() / 4);
			//sum = ParallelAlgorithms::SumParallelNonRecursive(u64Array, 0, uints.size(), sum_array);
			//sum = ParallelAlgorithms::SumParallelNonRecursive(u64Array.data(), 0, uints.size());
			//sum = ParallelAlgorithms::SumParallelNonRecursiveBuffered(u64Array, 0, uints.size());
			sum = ParallelAlgorithms::SumParallel(u64Array.data(), 0, uints.size());
			//sum = ParallelAlgorithms::SumParallel(u64Array, 0, uints.size(), uints.size() / 4);
			//sum = ParallelAlgorithms::SumParallel(u64Array, 0, uints.size(), uints.size() / 16);	// highest performance with /15 and /17 at half the performance

			endTime = high_resolution_clock::now();
			thruputs[j]  = (double)uints.size() / (duration_cast<duration<double, milli>>(endTime - startTime).count() / 1000.0) / 1000000.0;
			thruput_sum += (double)uints.size() / (duration_cast<duration<double, milli>>(endTime - startTime).count() / 1000.0) / 1000000.0;
			if (sum != sum_ref)
			{
				printf("Sums are not equal\n");
				exit(1);
			}
		}
		print_results("Parallel 64-bit Sum", sum, uints.size(), startTime, endTime, thruput_sum / num_times, std_deviation(thruputs));
	}
	return 0;
}
