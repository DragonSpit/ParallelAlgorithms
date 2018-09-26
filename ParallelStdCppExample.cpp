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
	high_resolution_clock::time_point endTime) {
	printf("%s: Lowest: %g Highest: %g Time: %fms\n", tag, sorted.front(),
		sorted.back(),
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

	return 0;
}
