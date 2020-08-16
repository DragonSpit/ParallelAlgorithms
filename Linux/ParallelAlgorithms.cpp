// ParallelAlgorithms.cpp : Linux benchmark of standard C++ parallel algorithms versus my implementations
//

#include <stddef.h>
#include <stdio.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <ratio>
#include <vector>
#include <execution>

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;
using std::random_device;
using std::sort;
using std::vector;

const int iterationCount = 5;

void print_results(const char *const tag, const vector<double>& sorted,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime)
{
	printf("%s: Lowest: %g Highest: %g Time: %fms\n", tag, sorted.front(), sorted.back(),
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

void print_results(const char* const tag, const vector<unsigned long>& sorted,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime)
{
	printf("%s: Lowest: %lu Highest: %lu Time: %fms\n", tag, sorted.front(), sorted.back(),
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

void print_results(const char* const tag, const vector<unsigned>& sorted,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime)
{
	printf("%s: Lowest: %u Highest: %u Time: %fms\n", tag, sorted.front(), sorted.back(),
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

void print_results(const char *const tag, double first, double last,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime)
{
	printf("%s: Lowest: %g Highest: %g Time: %fms\n", tag, first, last,
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

int ParallelStdCppExample(vector<double>& doubles)
{
	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		vector<double> sorted(doubles);
		const auto startTime = high_resolution_clock::now();
		sort(sorted.begin(), sorted.end());
		const auto endTime = high_resolution_clock::now();
		print_results("Serial", sorted, startTime, endTime);
	}

	for (int i = 0; i < iterationCount; ++i)
	{
		vector<double> sorted(doubles);
		const auto startTime = high_resolution_clock::now();
		// same sort call as above, but with par_unseq:
		sort(std::execution::par_unseq, sorted.begin(), sorted.end());
		const auto endTime = high_resolution_clock::now();
		// in our output, note that these are the parallel results:
		print_results("Parallel", sorted, startTime, endTime);
	}

	for (int i = 0; i < iterationCount; ++i)
	{
		double * s = new double[doubles.size()];
		for (unsigned int j = 0; j < doubles.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			s[j] = doubles[j];
		}
		const auto startTime = high_resolution_clock::now();
		sort(std::execution::par_unseq, s, s+doubles.size());
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Array", s[0], s[doubles.size() - 1], startTime, endTime);
		delete[] s;
	}

	return 0;
}

int ParallelStdCppExample(vector<unsigned long>& ulongs)
{
	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		vector<unsigned long> sorted(ulongs);
		const auto startTime = high_resolution_clock::now();
		sort(sorted.begin(), sorted.end());
		const auto endTime = high_resolution_clock::now();
		print_results("Serial", sorted, startTime, endTime);
	}

	for (int i = 0; i < iterationCount; ++i)
	{
		vector<unsigned long> sorted(ulongs);
		const auto startTime = high_resolution_clock::now();
		// same sort call as above, but with par_unseq:
		sort(std::execution::par_unseq, sorted.begin(), sorted.end());
		const auto endTime = high_resolution_clock::now();
		// in our output, note that these are the parallel results:
		print_results("Parallel", sorted, startTime, endTime);
	}

	for (int i = 0; i < iterationCount; ++i)
	{
		unsigned long* s = new unsigned long[ulongs.size()];
		for (unsigned int j = 0; j < ulongs.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			s[j] = ulongs[j];
		}
		const auto startTime = high_resolution_clock::now();
		sort(std::execution::par_unseq, s, s + ulongs.size());
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Array", s[0], s[ulongs.size() - 1], startTime, endTime);
		delete[] s;
	}

	return 0;
}

int ParallelStdCppExample(vector<unsigned>& uints)
{
	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		vector<unsigned> sorted(uints);
		const auto startTime = high_resolution_clock::now();
		sort(sorted.begin(), sorted.end());
		const auto endTime = high_resolution_clock::now();
		print_results("Serial", sorted, startTime, endTime);
	}

	for (int i = 0; i < iterationCount; ++i)
	{
		vector<unsigned> sorted(uints);
		const auto startTime = high_resolution_clock::now();
		// same sort call as above, but with par_unseq:
		sort(std::execution::par_unseq, sorted.begin(), sorted.end());
		const auto endTime = high_resolution_clock::now();
		// in our output, note that these are the parallel results:
		print_results("Parallel", sorted, startTime, endTime);
	}

	for (int i = 0; i < iterationCount; ++i)
	{
		unsigned *s = new unsigned[uints.size()];
		for (unsigned int j = 0; j < uints.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			s[j] = uints[j];
		}
		const auto startTime = high_resolution_clock::now();
		sort(std::execution::par_unseq, s, s + uints.size());
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Array", s[0], s[uints.size() - 1], startTime, endTime);
		delete[] s;
	}

	return 0;
}

int main()
{
	// Provide the same input random array of doubles to all sorting algorithms
	const size_t testSize = 10'000'000;
	random_device rd;

	// generate some random doubles:
	printf("Testing with %zu doubles...\n", testSize);
	vector<double> doubles(testSize);
	for (auto& d : doubles) {
		d = static_cast<double>(rd());
	}
	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(doubles);

	// generate some random unsigned longs:
	printf("\nTesting with %zu unsigned longs...\n", testSize);
	vector<unsigned long> ulongs(testSize);
	for (auto& d : ulongs) {
		d = static_cast<unsigned long>(rd());
	}
	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(ulongs);

	// generate some random unsigned integers:
	printf("\nTesting with %zu unsigned integers...\n", testSize);
	vector<unsigned> uints(testSize);
	for (auto& d : uints) {
		d = static_cast<unsigned>(rd());
	}
	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(uints);

	return 0;
}
