// ParallelAlgorithms main application entry point
#if 0

#include <oneapi/dpl/execution>
#include <oneapi/dpl/algorithm>

#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <ratio>
#include <vector>
#include <execution>

using std::random_device;
using std::vector;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;
using std::random_device;
using std::sort;
using std::vector;

static const int iterationCount = 1000;

static void print_results(const char* const tag, const vector<unsigned>& sorted, high_resolution_clock::time_point startTime, high_resolution_clock::time_point endTime)
{
	printf("%s: Lowest: %u Highest: %u Time: %fms\n", tag, sorted.front(), sorted.back(),
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

static int ParallelStdCppExample(vector<unsigned>& uints, bool stable = false)
{
	for (int i = 0; i < iterationCount; ++i)
	{
		vector<unsigned> sorted(uints);
		const auto startTime = high_resolution_clock::now();
		// same sort call as above, but with par_unseq:
		if (!stable)
		{
			//sort(std::execution::par_unseq, sorted.begin(), sorted.end()); 
			sort(oneapi::dpl::execution::par_unseq, sorted.begin(), sorted.end());
		}
		else
		{
			//stable_sort(std::execution::par_unseq, sorted.begin(), sorted.end());
			stable_sort(oneapi::dpl::execution::par_unseq, sorted.begin(), sorted.end());
		}
		const auto endTime = high_resolution_clock::now();
		// in our output, note that these are the parallel results:
		print_results("Parallel", sorted, startTime, endTime);
	}

	return 0;
}


//int main()
int std_parallel_sort_leak_demo()
{
	// Test configuration options
	bool UseStableStdSort = false;

	// Provide the same input random array of doubles to all sorting algorithms
	const size_t testSize = 1'000'000'000;
	//random_device rd;
	std::mt19937_64 dist(1234);

	// generate some random unsigned integers:
	printf("\nTesting with %zu random unsigned integers...\n\n", testSize);
	vector<unsigned> uints(testSize);
	for (auto& d : uints) {
		//d = static_cast<unsigned>(rd());
		d = static_cast<unsigned>(dist());   // way faster on Linux
	}
	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(uints, UseStableStdSort);

	return 0;
}
#endif