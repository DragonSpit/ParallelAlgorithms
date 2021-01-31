// From https://blogs.msdn.microsoft.com/vcblog/2018/09/11/using-c17-parallel-algorithms-for-better-performance/
// compile with:
//  debug: cl /EHsc /W4 /WX /std:c++latest /Fedebug /MDd .\program.cpp
//  release: cl /EHsc /W4 /WX /std:c++latest /Ferelease /MD /O2 .\program.cpp
#include <stddef.h>
#include <stdio.h>
#include <algorithm>
#include <chrono>
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

	printf("\nAccumulate/Sum Benchmark:\n");

	for (int i = 0; i < iterationCount; ++i)
	{
		unsigned* s = new unsigned[uints.size()];
		for (unsigned int j = 0; j < uints.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			s[j] = uints[j];
		}
		const auto startTime = high_resolution_clock::now();
		unsigned result_serial = std::accumulate(s, s + uints.size(), 0);
		const auto endTime = high_resolution_clock::now();
		print_results("Serial Array Sum", result_serial, result_serial, startTime, endTime);
		delete[] s;
	}
	for (int i = 0; i < iterationCount; ++i)
	{
		unsigned* s = new unsigned[uints.size()];
		for (unsigned int j = 0; j < uints.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			s[j] = uints[j];
		}
		const auto startTime = high_resolution_clock::now();
		unsigned result_parallel = std::reduce(std::execution::par, s, s + uints.size(), 0);		//Faster
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Array Sum", result_parallel, result_parallel, startTime, endTime);
		delete[] s;
	}

	return 0;
}
