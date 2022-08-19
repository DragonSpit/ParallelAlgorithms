#pragma once

// Counting Sort implementations

#ifndef _CountingSort_h
#define _CountingSort_h

//#include <cstddef>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <thread>
#include <execution>
#include <ppl.h>
#else
#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <ratio>
#include <vector>
#include <thread>
#include <execution>
#endif

namespace ParallelAlgorithms
{
	// left-inclusive and right-exclusive boundaries
	inline void counting_sort(unsigned char* array_to_sort, size_t l, size_t r)
	{
		// TODO: Turn this into Histogram of Byte, as it's a useful abstraction for counting bytes of a byte array
		size_t counts[256]{};
		for (size_t _current = l; _current < r; _current++)	    // Scan the array and count the number of times each value appears
			counts[array_to_sort[_current]]++;

		size_t start_index = 0;
		for (size_t count_index = 0; count_index < 256; count_index++)
		{
#if 1
			memset(array_to_sort + start_index, count_index, counts[count_index]);
#else
			size_t end_index = start_index + counts[count_index];
			for (size_t i = start_index; i <= end_index; i++)
				array_to_sort[i] = count_index;
#endif
			start_index += counts[count_index];
		}
	}
}
#endif