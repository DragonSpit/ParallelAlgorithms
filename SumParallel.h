// TODO: Implement a more efficient suggestion of using task_group to split the array into chunks with each returning its sum into one index of an array of sums
#pragma once

// Parallel Sum implementations

#ifndef _SumParallel_h
#define _SumParallel_h

#include "Configuration.h"

#include <thread>
#include <execution>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <ratio>
#include <vector>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
  #define __TBB_PREVIEW_TASK_ARENA_CONSTRAINTS_EXTENSION_PRESENT 1
  #include <oneapi/tbb/task_arena.h>
#endif

#include "RadixSortMsdParallel.h"
#include "FillParallel.h"

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;
using std::random_device;
using std::sort;
using std::vector;


namespace ParallelAlgorithms
{
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long Sum(unsigned long long in_array[], size_t l, size_t r)
	{
		unsigned long long sum = 0;
		for (size_t current = l; current < r; current++)
			sum += in_array[current];
		//unsigned long long sum_left = std::accumulate(in_array + l, in_array + r, 0);	// may be implemented using SIMD/SSE
		return sum;
	}

	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long Sum(unsigned in_array[], size_t l, size_t r)
	{
		unsigned long long sum = 0;
		for (size_t current = l; current < r; current++)
			sum += (unsigned long long)in_array[current];
		//unsigned long long sum_left = std::accumulate(in_array + l, in_array + r, 0);	// may be implemented using SIMD/SSE
		return sum;
	}

	// left (l) boundary is inclusive and right (r) boundary is exclusive
	// TODO: Not yet implemented
	inline unsigned long long SumUnrolled(unsigned in_array[], size_t l, size_t r)
	{
		unsigned long long sum = 0;
		for (size_t current = l; current < r; current++)
			sum += (unsigned long long)in_array[current];
		//unsigned long long sum_left = std::accumulate(in_array + l, in_array + r, 0);	// may be implemented using SIMD/SSE
		return sum;
	}
#if 0
	size_t last_by_four = l + ((r - l) / 4) * 4;
	size_t current = l;
	for (; current < last_by_four;)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
	{
		countLeft_0[inArray[current]]++;  current++;
		countLeft_1[inArray[current]]++;  current++;
		countLeft_2[inArray[current]]++;  current++;
		countLeft_3[inArray[current]]++;  current++;
	}
#endif
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long SumBufferedLocally(unsigned in_array[], size_t l, size_t r)
	{
		const size_t BUFFER_DEPTH = 1024;
		unsigned buffer_loc[BUFFER_DEPTH];
		size_t num_buffers = (r - l + (BUFFER_DEPTH - 1)) / BUFFER_DEPTH;
		unsigned long long sum = 0;
		size_t current = l;
		size_t i = 0;
		for (; i < (num_buffers - 1); ++i)
		{
			std::copy(in_array + current, in_array + current + BUFFER_DEPTH, buffer_loc);	// possibly using SIMD/SSE. TODO: Need to switch to Intel's unseq version
			for (size_t j = 0; j < BUFFER_DEPTH; ++j)
				sum += buffer_loc[j];
			current += BUFFER_DEPTH;
		}
		for (; current < r; ++current)
			sum += in_array[current];
		return sum;
	}

	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long SumBufferedLocally(unsigned long long in_array[], size_t l, size_t r)
	{
		const size_t BUFFER_DEPTH = 1024;
		unsigned long long buffer_loc[BUFFER_DEPTH];
		size_t num_buffers = (r - l + (BUFFER_DEPTH - 1)) / BUFFER_DEPTH;
		unsigned long long sum = 0;
		size_t current = l;
		size_t i = 0;
		for (; i < (num_buffers - 1); ++i)
		{
			std::copy(in_array + current, in_array + current + BUFFER_DEPTH, buffer_loc);	// possibly using SIMD/SSE. TODO: Need to switch to Intel's unseq version
			for (size_t j = 0; j < BUFFER_DEPTH; ++j)
				sum += buffer_loc[j];
			current += BUFFER_DEPTH;
		}
		for (; current < r; ++current)
			sum += in_array[current];
		return sum;
	}

	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long SumBufferedExternally(unsigned in_array[], size_t l, size_t r, unsigned buffer[], size_t buffer_depth)
	{
		size_t num_buffers = (r - l + (buffer_depth - 1)) / buffer_depth;
		unsigned long long sum = 0;
		size_t current = l;
		size_t i = 0;
		for (; i < (num_buffers - 1); ++i)
		{
			std::copy(in_array + current, in_array + current + buffer_depth, buffer);	// possibly using SIMD/SSE. TODO: Need to switch to Intel's unseq version
			for (size_t j = 0; j < buffer_depth; ++j)
				sum += buffer[j];
			current += buffer_depth;
		}
		for (; current < r; ++current)
			sum += in_array[current];
		return sum;
	}

	// left (l) boundary is inclusive and right (r) boundary is exclusive
	// 50% slower than non-buffered version
	inline unsigned long long SumBufferedExternally(unsigned long long in_array[], size_t l, size_t r, unsigned long long buffer[], size_t buffer_depth)
	{
		size_t num_buffers = (r - l + (buffer_depth - 1)) / buffer_depth;
		unsigned long long sum = 0;
		size_t current = l;
		size_t i = 0;
		for (; i < (num_buffers - 1); ++i)
		{
			std::copy(in_array + current, in_array + current + buffer_depth, buffer);	// possibly using SIMD/SSE. TODO: Need to switch to Intel's unseq version
			for (size_t j = 0; j < buffer_depth; ++j)
				sum += buffer[j];
			current += buffer_depth;
		}
		for (; current < r; ++current)
			sum += in_array[current];
		return sum;
	}

	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long SumParallel(unsigned long long in_array[], size_t l, size_t r, size_t parallelThreshold = 16 * 1024)
	{
		//if (((unsigned long long)(in_array + l) & 0x7) != 0)
		//	printf("Memory alignment is not on 8-byte boundary\n");
		if ((r - l) <= parallelThreshold)
			return Sum( in_array, l, r );
			//return std::accumulate(in_array + l, in_array + r, 0ULL);

		unsigned long long sum_left = 0, sum_right = 0;

		size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;  // average without overflow

#if defined(USE_PPL)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { sum_left  = SumParallel(in_array, l, m, parallelThreshold); },
			[&] { sum_right = SumParallel(in_array, m, r, parallelThreshold); }
		);
		// Combine left and right results
		sum_left += sum_right;

		return sum_left;
	}
#if 0
	inline unsigned long long SumParallel(unsigned long long in_array[], size_t l, size_t r, size_t parallelThreshold = 16 * 1024)
	{
		//may return 0 when not able to detect
		auto processor_count = std::thread::hardware_concurrency();
		if (processor_count < 1)
		{
			processor_count = 1;
			//cout << "Warning: Fewer than 1 processor core detected. Using only a single core.";
		}

		size_t length = r - l + 1;

		if ((parallelThreshold * processor_count) < length)
			parallelThreshold = length / processor_count;
		return SumParallel_inner(in_array, l, r, parallelThreshold);
	}
#endif
	// Sum of an arbitrary numerical type to a 64-bit sum
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	template< class _Type >
	inline long long SumParallel(_Type in_array[], size_t l, size_t r, size_t parallelThreshold = 16 * 1024)
	{
		//if (((unsigned long long)(in_array + l) & 0x7) != 0)
		//	printf("Memory alignment is not on 8-byte boundary\n");
		if ((r - l) <= parallelThreshold)
		{
			long long sum_left = 0;
			for (size_t current = l; current < r; current++)
				sum_left += (long long)in_array[current];
			//long long sum_left = std::accumulate(in_array + l, in_array + r, 0LL);
			return sum_left;
		}

		long long sum_left = 0, sum_right = 0;

		size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;  // average without overflow

#if defined(USE_PPL)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { sum_left  = SumParallel(in_array, l, m, parallelThreshold); },
			[&] { sum_right = SumParallel(in_array, m, r, parallelThreshold); }
		);
		// Combine left and right results
		sum_left += sum_right;

		return sum_left;
	}
	// Non-recursive Sum
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long SumNonRecursive(unsigned long long in_array[], size_t l, size_t r, size_t parallelThreshold = 128 * 1024)
	{
		size_t num_tasks = (r - l + (parallelThreshold - 1)) / parallelThreshold;
		unsigned long long* sum_array = new unsigned long long[num_tasks] {};

		size_t i = 0;
		for (; i < (num_tasks - 1); i++)
			sum_array[i] = Sum(in_array, l + parallelThreshold * i, l + parallelThreshold * (i + 1));	// process full parallelThreshold chunks

		sum_array[num_tasks - 1] = Sum(in_array, l + parallelThreshold * i, r);	// process the last partial parallelThreshold chunk

		unsigned long long sum = 0;
		for (size_t i = 0; i < num_tasks; i++)
			sum += sum_array[i];

		delete[] sum_array;
		return sum;
	}

	// Non-recursive Parallel Sum
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long SumParallelNonRecursive(unsigned in_array[], size_t l, size_t r, size_t parallelThreshold = 16 * 1024)
	{
		size_t num_tasks = (r - l + (parallelThreshold - 1)) / parallelThreshold;
		unsigned long long* sum_array = new unsigned long long[num_tasks] {};
		tbb::task_group g;

		size_t i = 0;
		for (; i < (num_tasks - 1); i++)
			g.run([=] {sum_array[i] = Sum(in_array, l + parallelThreshold * i, l + parallelThreshold * (i + 1)); });	// process full parallelThreshold chunks

		g.run([=] {sum_array[num_tasks - 1] = Sum(in_array, l + parallelThreshold * i, r); });	// process the last partial parallelThreshold chunk

		g.wait();	// wait for all tasks to complete

		unsigned long long sum = 0;
		for (size_t i = 0; i < num_tasks; i++)
			sum += sum_array[i];

		delete[] sum_array;
		return sum;
	}

	// Non-recursive Parallel Sum
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long SumParallelNonRecursive(unsigned long long in_array[], size_t l, size_t r, size_t parallelThreshold = 16 * 1024)
	{
		size_t num_tasks = (r - l + (parallelThreshold - 1)) / parallelThreshold;
		unsigned long long* sum_array = new unsigned long long[num_tasks] {};
		tbb::task_group g;

		size_t i = 0;
		for (; i < (num_tasks - 1); i++)
			g.run([=] {sum_array[i] = Sum(in_array, l + parallelThreshold * i, l + parallelThreshold * (i + 1)); });	// process full parallelThreshold chunks

		g.run([=] {sum_array[num_tasks - 1] = Sum(in_array, l + parallelThreshold * i, r); });	// process the last partial parallelThreshold chunk

		g.wait();	// wait for all tasks to complete

		unsigned long long sum = 0;
		for (size_t i = 0; i < num_tasks; i++)
			sum += sum_array[i];

		delete[] sum_array;
		return sum;
	}

	// Non-recursive Parallel Sum
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long SumParallelNonRecursiveBufferedLocally(unsigned in_array[], size_t l, size_t r, size_t parallelThreshold = 16 * 1024)
	{
		size_t num_tasks = (r - l + (parallelThreshold - 1)) / parallelThreshold;
		unsigned long long* sum_array = new unsigned long long[num_tasks] {};
		tbb::task_group g;

		size_t i = 0;
		for (; i < (num_tasks - 1); i++)
			g.run([=] {		// process full parallelThreshold chunks
				sum_array[i] = SumBufferedLocally(in_array, l + parallelThreshold * i, l + parallelThreshold * (i + 1));
				});

		g.run([=] {			// process the last partial parallelThreshold chunk
			sum_array[num_tasks - 1] = SumBufferedLocally(in_array, l + parallelThreshold * i, r);
			});

		g.wait();	// wait for all tasks to complete

		unsigned long long sum = 0;
		for (size_t i = 0; i < num_tasks; i++)
			sum += sum_array[i];

		delete[] sum_array;
		return sum;
	}

	// Non-recursive Parallel Sum
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long SumParallelNonRecursiveBufferedLocally(unsigned long long in_array[], size_t l, size_t r, size_t parallelThreshold = 16 * 1024)
	{
		size_t num_tasks = (r - l + (parallelThreshold - 1)) / parallelThreshold;
		unsigned long long* sum_array = new unsigned long long[num_tasks] {};
		const size_t BUFFER_DEPTH_PER_TASK = 1024;
		tbb::task_group g;

		size_t i = 0;
		for (; i < (num_tasks - 1); i++)
			g.run([=] {		// process full parallelThreshold chunks
				sum_array[i] = SumBufferedLocally(in_array, l + parallelThreshold * i, l + parallelThreshold * (i + 1));
				});

		g.run([=] {			// process the last partial parallelThreshold chunk
			sum_array[num_tasks - 1] = SumBufferedLocally(in_array, l + parallelThreshold * i, r);
			});

		g.wait();	// wait for all tasks to complete

		unsigned long long sum = 0;
		for (size_t i = 0; i < num_tasks; i++)
			sum += sum_array[i];

		delete[] sum_array;
		return sum;
	}

	// Non-recursive Parallel Sum
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long SumParallelNonRecursiveBuffered(unsigned in_array[], size_t l, size_t r, size_t parallelThreshold = 16 * 1024)
	{
		size_t num_tasks = (r - l + (parallelThreshold - 1)) / parallelThreshold;
		unsigned long long* sum_array = new unsigned long long[num_tasks] {};
		const size_t BUFFER_DEPTH_PER_TASK = 1024;
		unsigned* buffers = new unsigned[num_tasks * BUFFER_DEPTH_PER_TASK] {};
		tbb::task_group g;

		size_t i = 0;
		for (; i < (num_tasks - 1); i++)
			g.run([=] {sum_array[i] = SumBufferedExternally(in_array, l + parallelThreshold * i, l + parallelThreshold * (i + 1), &buffers[i * BUFFER_DEPTH_PER_TASK], BUFFER_DEPTH_PER_TASK); });	// process full parallelThreshold chunks

		g.run([=] {sum_array[num_tasks - 1] = Sum(in_array, l + parallelThreshold * i, r); });	// process the last partial parallelThreshold chunk

		g.wait();	// wait for all tasks to complete

		unsigned long long sum = 0;
		for (size_t i = 0; i < num_tasks; i++)
			sum += sum_array[i];

		delete[] sum_array;
		return sum;
	}

	// Non-recursive Parallel Sum
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	// 50% slower than non-buffered version
	inline unsigned long long SumParallelNonRecursiveBuffered(unsigned long long in_array[], size_t l, size_t r, size_t parallelThreshold = 16 * 1024)
	{
		size_t num_tasks = (r - l + (parallelThreshold - 1)) / parallelThreshold;
		unsigned long long* sum_array = new unsigned long long[num_tasks] {};
		const size_t BUFFER_DEPTH_PER_TASK = 1024;
		unsigned long long* buffers = new unsigned long long[num_tasks * BUFFER_DEPTH_PER_TASK] {};
		tbb::task_group g;

		size_t i = 0;
		for (; i < (num_tasks - 1); i++)
			g.run([=] {sum_array[i] = SumBufferedExternally(in_array, l + parallelThreshold * i, l + parallelThreshold * (i + 1), &buffers[i * BUFFER_DEPTH_PER_TASK], BUFFER_DEPTH_PER_TASK); });	// process full parallelThreshold chunks

		g.run([=] {sum_array[num_tasks - 1] = Sum(in_array, l + parallelThreshold * i, r); });	// process the last partial parallelThreshold chunk

		g.wait();	// wait for all tasks to complete

		unsigned long long sum = 0;
		for (size_t i = 0; i < num_tasks; i++)
			sum += sum_array[i];

		delete[] sum_array;
		return sum;
	}

	// Non-recursive Parallel Sum
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long SumParallelNonRecursive(unsigned in_array[], size_t l, size_t r, unsigned long long* sum_array, size_t parallelThreshold = 16 * 1024)
	{
		size_t num_tasks = (r - l + (parallelThreshold - 1)) / parallelThreshold;
		//unsigned long long* sum_array = new unsigned long long[num_tasks] {};
		tbb::task_group g;

		size_t i = 0;
		for (; i < (num_tasks - 1); i++)
			g.run([=] {sum_array[i] = Sum(in_array, l + parallelThreshold * i, l + parallelThreshold * (i + 1)); });	// process full parallelThreshold chunks

		g.run([=] {sum_array[num_tasks - 1] = Sum(in_array, l + parallelThreshold * i, r); });	// process the last partial parallelThreshold chunk

		g.wait();	// wait for all tasks to complete

		unsigned long long sum = 0;
		for (size_t i = 0; i < num_tasks; i++)
			sum += sum_array[i];

		return sum;
	}

	// Non-recursive Parallel Sum
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long SumParallelNonRecursive(unsigned long long in_array[], size_t l, size_t r, unsigned long long* sum_array, size_t parallelThreshold = 16 * 1024)
	{
		size_t num_tasks = (r - l + (parallelThreshold - 1)) / parallelThreshold;
		//unsigned long long* sum_array = new unsigned long long[num_tasks] {};
		tbb::task_group g;

		size_t i = 0;
		for (; i < (num_tasks - 1); i++)
			g.run([=] {sum_array[i] = Sum(in_array, l + parallelThreshold * i, l + parallelThreshold * (i + 1)); });	// process full parallelThreshold chunks

		g.run([=] {sum_array[num_tasks - 1] = Sum(in_array, l + parallelThreshold * i, r); });	// process the last partial parallelThreshold chunk

		g.wait();	// wait for all tasks to complete

		unsigned long long sum = 0;
		for (size_t i = 0; i < num_tasks; i++)
			sum += sum_array[i];

		return sum;
	}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	// Non-recursive Parallel Sum without Hyperthreading
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long SumParallelNonRecursiveNoHyperthreading(unsigned long long in_array[], size_t l, size_t r, size_t parallelThreshold = 16 * 1024)
	{
		size_t num_tasks = (r - l + (parallelThreshold - 1)) / parallelThreshold;
		unsigned long long* sum_array = new unsigned long long[num_tasks] {};

		int no_ht_concurrency = tbb::info::default_concurrency(
			tbb::task_arena::constraints{}.set_max_threads_per_core(1)
		);
		tbb::task_arena arena(no_ht_concurrency);
		arena.execute([=] {
			tbb::task_group g;
			size_t i = 0;
			for (; i < (num_tasks - 1); i++)
				g.run([=] {sum_array[i] = Sum(in_array, l + parallelThreshold * i, l + parallelThreshold * (i + 1)); });	// process full parallelThreshold chunks

			g.run([=] {sum_array[num_tasks - 1] = Sum(in_array, l + parallelThreshold * i, r); });	// process the last partial parallelThreshold chunk

			g.wait();	// wait for all tasks to complete
		});

		unsigned long long sum = 0;
		for (size_t i = 0; i < num_tasks; i++)
			sum += sum_array[i];

		delete[] sum_array;
		return sum;
	}

	// Non-recursive Parallel Sum without Hyperthreading
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long SumParallelNonRecursiveNoHyperthreading(unsigned in_array[], size_t l, size_t r, size_t parallelThreshold = 16 * 1024)
	{
		size_t num_tasks = (r - l + (parallelThreshold - 1)) / parallelThreshold;
		unsigned long long* sum_array = new unsigned long long[num_tasks] {};

		int no_ht_concurrency = tbb::info::default_concurrency(
			tbb::task_arena::constraints{}.set_max_threads_per_core(1)
		);
		tbb::task_arena arena(no_ht_concurrency);
		arena.execute([=] {
			tbb::task_group g;
			size_t i = 0;
			for (; i < (num_tasks - 1); i++)
				g.run([=] {sum_array[i] = Sum(in_array, l + parallelThreshold * i, l + parallelThreshold * (i + 1)); });	// process full parallelThreshold chunks

			g.run([=] {sum_array[num_tasks - 1] = Sum(in_array, l + parallelThreshold * i, r); });	// process the last partial parallelThreshold chunk

			g.wait();	// wait for all tasks to complete
			});

		unsigned long long sum = 0;
		for (size_t i = 0; i < num_tasks; i++)
			sum += sum_array[i];

		delete[] sum_array;
		return sum;
	}

	// Non-recursive Parallel Sum without Hyperthreading
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	inline unsigned long long SumParallelNonRecursiveBufferedLocallyNoHyperthreading(
		unsigned in_array[], size_t l, size_t r, size_t parallelThreshold = 16 * 1024)
	{
		size_t num_tasks = (r - l + (parallelThreshold - 1)) / parallelThreshold;
		unsigned long long* sum_array = new unsigned long long[num_tasks] {};

		int no_ht_concurrency = tbb::info::default_concurrency(
			tbb::task_arena::constraints{}.set_max_threads_per_core(1)
		);
		tbb::task_arena arena(no_ht_concurrency);
		arena.execute([=] {
			tbb::task_group g;
			size_t i = 0;
			for (; i < (num_tasks - 1); i++)
				g.run([=] {sum_array[i] = SumBufferedLocally(in_array, l + parallelThreshold * i, l + parallelThreshold * (i + 1)); });	// process full parallelThreshold chunks

			g.run([=] {sum_array[num_tasks - 1] = SumBufferedLocally(in_array, l + parallelThreshold * i, r); });	// process the last partial parallelThreshold chunk

			g.wait();	// wait for all tasks to complete
			});

		unsigned long long sum = 0;
		for (size_t i = 0; i < num_tasks; i++)
			sum += sum_array[i];

		delete[] sum_array;
		return sum;
	}
#endif
}

#endif