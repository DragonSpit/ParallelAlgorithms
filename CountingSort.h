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

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;
using std::random_device;
using std::sort;
using std::vector;

void print_results(const char* const tag, high_resolution_clock::time_point startTime, high_resolution_clock::time_point endTime)
{
	printf("%s: Time: %fms\n", tag,	duration_cast<duration<double, milli>>(endTime - startTime).count());
}

namespace ParallelAlgorithms
{
	// left-inclusive and right-exclusive boundaries
	inline void counting_sort(unsigned char* array_to_sort, size_t l, size_t r)
	{
		//const auto startTimeHistogram = high_resolution_clock::now();

		// TODO: Turn this into Histogram of Byte, as it's a useful abstraction for counting bytes of a byte array
		size_t counts[256]{};
		for (size_t _current = l; _current < r; _current++)	    // Scan the array and count the number of times each value appears
			counts[array_to_sort[_current]]++;

		//const auto endTimeHistogram = high_resolution_clock::now();
		//print_results("Histogram inside byte array Counting Sort", startTimeHistogram, endTimeHistogram);

		//const auto startTime = high_resolution_clock::now();

		size_t start_index = 0;
		for (size_t count_index = 0; count_index < 256; count_index++)
		{
#if 1
			//memset(array_to_sort + start_index, count_index, counts[count_index]);
			std::fill(array_to_sort + start_index, array_to_sort + start_index + counts[count_index], (unsigned char)count_index);
#else
			size_t end_index = start_index + counts[count_index];
			for (size_t i = start_index; i <= end_index; i++)
				array_to_sort[i] = count_index;
#endif
			start_index += counts[count_index];
		}
		//const auto endTime = high_resolution_clock::now();
		//print_results("Fill inside byte array Counting Sort", startTime, endTime);
	}
#include <windows.h>

	void ErrorExit()
	{
		// Retrieve the system error message for the last-error code

		LPVOID lpMsgBuf;
		DWORD dw = GetLastError();

		if (FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL) == 0) {
			MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_OK);
			ExitProcess(dw);
		}

		MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK);

		LocalFree(lpMsgBuf);
		ExitProcess(dw);
	}

	// VirtualAlloc version of counting sort, using large pages.
	// left-inclusive and right-exclusive boundaries
	inline void counting_sort_v(unsigned int* array_to_sort, size_t l, size_t r)
	{
		//const auto startTimeHistogram = high_resolution_clock::now();
		std::cout << "Counting Sort algorithm with VirtualAlloc for huge pages" << std::endl;

		size_t COUNT_ARRAY_SIZE = (size_t)1 << (sizeof(unsigned int) * 8);  // tied to the type of the input array - i.e. index of the count array is the array element value
		size_t SIZE_OF_EACH_COUNT = sizeof(size_t);  // 64-bits for any size of array, but could be reduced to 32-bits for arrays smaller than 4 billion elements (future optimization)
		size_t* counts_p;
		std::cout << "Large page minimum size = " << GetLargePageMinimum() << std::endl; // seems to be 2MB on Windows 11 Pro
#if 1
		counts_p = (size_t*)VirtualAlloc(NULL, COUNT_ARRAY_SIZE * SIZE_OF_EACH_COUNT, MEM_COMMIT | MEM_RESERVE | MEM_LARGE_PAGES, PAGE_READWRITE); // Error: "The required privilege is not held by the client" To enable this privilege, use the Local Security Policy snap-in (secpol.msc) and enable the "Lock pages in memory" user right for the account that will run this code.
		//counts_p = (size_t*)VirtualAlloc(NULL, 2 * 1024 * 1024, MEM_COMMIT | MEM_RESERVE | MEM_LARGE_PAGES, PAGE_READWRITE); // Error: "The required privilege is not held by the client" To enable this privilege, use the Local Security Policy snap-in (secpol.msc) and enable the "Lock pages in memory" user right for the account that will run this code.
		// TODO: The executable needs to be run with admin privileges, and the user account running the executable needs to have the "Lock pages in memory" privilege enabled in the Local Security Policy snap-in (secpol.msc) to use large pages. If these conditions are not met, VirtualAlloc will fail with the error "The required privilege is not held by the client". To enable this privilege, open the Local Security Policy snap-in (secpol.msc), navigate to Local Policies > User Rights Assignment, and add the user account that will run this code to the "Lock pages in memory" policy.
		//       Try smaller amount of memory to see if it works without admin privileges, e.g. 1 million counts (4MB) instead of 4 billion counts (16GB).
		//       Chaotica Windows Application showed that large pages are enabled!
#else
#pragma comment(lib, "mincore.lib")
		MEM_EXTENDED_PARAMETER extended{};
		extended.Type = MemExtendedParameterAttributeFlags;
		extended.ULong64 = MEM_EXTENDED_PARAMETER_NONPAGED_HUGE;

		counts_p = (size_t*)VirtualAlloc2(GetCurrentProcess(), NULL, COUNT_ARRAY_SIZE * SIZE_OF_EACH_COUNT,
			MEM_LARGE_PAGES | MEM_RESERVE | MEM_COMMIT,
			PAGE_READWRITE, &extended, 1);
#endif
		if (counts_p == NULL)
			ErrorExit();
		//counts_p = (size_t*)VirtualAlloc(NULL, COUNT_ARRAY_SIZE * SIZE_OF_EACH_COUNT, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		std::fill_n(counts_p, COUNT_ARRAY_SIZE, 0);	// zero out the counts array
		for (size_t _current = l; _current < r; _current++)	    // Scan the array and count the number of times each value appears
			counts_p[array_to_sort[_current]]++;

		//const auto endTimeHistogram = high_resolution_clock::now();
		//print_results("Histogram inside byte array Counting Sort", startTimeHistogram, endTimeHistogram);

		//const auto startTime = high_resolution_clock::now();

		size_t start_index = 0;
		for (size_t count_index = 0; count_index < COUNT_ARRAY_SIZE; count_index++)
		{
#if 0
			//memset(array_to_sort + start_index, count_index, counts[count_index]);
			std::fill(array_to_sort + start_index, array_to_sort + start_index + counts[count_index], (unsigned char)count_index);
#else
			size_t end_index = start_index + counts_p[count_index];
			for (size_t i = start_index; i <= end_index; i++)
				array_to_sort[i] = (unsigned int)count_index;
#endif
			start_index += counts_p[count_index];
		}
		VirtualFree(counts_p, 0, MEM_RELEASE);
		//const auto endTime = high_resolution_clock::now();
		//print_results("Fill inside byte array Counting Sort", startTime, endTime);
	}
	// left-inclusive and right-exclusive boundaries
	inline void counting_sort(unsigned int* array_to_sort, size_t l, size_t r)
	{
		//const auto startTimeHistogram = high_resolution_clock::now();

		if ((r - l >= ((size_t)1 << 32)))
			return counting_sort_v(array_to_sort, l, r);

		std::cout << "Counting Sort algorithm" << std::endl;

		size_t COUNT_ARRAY_SIZE = (size_t)1 << (sizeof(unsigned int) * 8);  // tied to the type of the input array - i.e. index of the count array is the array element value
		unsigned SIZE_OF_EACH_COUNT = sizeof(unsigned);  // 32-bits for arrays smaller than 4 billion elements
		unsigned* counts_p;
		counts_p = new unsigned[COUNT_ARRAY_SIZE * SIZE_OF_EACH_COUNT]();

		for (size_t _current = l; _current < r; _current++)	    // Scan the array and count the number of times each value appears
			counts_p[array_to_sort[_current]]++;

		//const auto endTimeHistogram = high_resolution_clock::now();
		//print_results("Histogram inside byte array Counting Sort", startTimeHistogram, endTimeHistogram);

		//const auto startTime = high_resolution_clock::now();

		size_t start_index = 0;
		for (size_t count_index = 0; count_index < COUNT_ARRAY_SIZE; count_index++)
		{
#if 0
			//memset(array_to_sort + start_index, count_index, counts[count_index]);
			std::fill(array_to_sort + start_index, array_to_sort + start_index + counts[count_index], (unsigned char)count_index);
#else
			size_t end_index = start_index + counts_p[count_index];
			for (size_t i = start_index; i <= end_index; i++)
				array_to_sort[i] = (unsigned int)count_index;
#endif
			start_index += counts_p[count_index];
		}
		delete[] counts_p;
		//const auto endTime = high_resolution_clock::now();
		//print_results("Fill inside byte array Counting Sort", startTime, endTime);
	}
}
#endif