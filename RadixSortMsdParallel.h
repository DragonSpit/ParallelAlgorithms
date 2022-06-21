#pragma once

#ifndef _RadixSortMSD_h
#define _RadixSortMSD_h

#include "InsertionSort.h"
#include "BinarySearch.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include "tbb/tbb.h"
#include <thread>
#include <ppl.h>
#else
#include <iostream>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <ratio>
#include <vector>
#include <execution>
#include <thread>
#include <tbb/task_group.h>
#include <tbb/parallel_invoke.h>
#include <string.h>
#endif

using namespace tbb;

#include "RadixSortCommon.h"
#include "InsertionSort.h"

template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline size_t* HistogramOneByteComponentParallel(unsigned long inArray[], size_t l, size_t r, unsigned long shiftRight, size_t parallelThreshold = 64 * 1024)
{
	const unsigned long numberOfBins = PowerOfTwoRadix;

	size_t* countLeft  = NULL;
	size_t* countRight = NULL;

	if (l > r)      // zero elements to compare
	{
		countLeft = new size_t[numberOfBins];
		for (size_t j = 0; j < numberOfBins; j++)
			countLeft[j] = 0;
		return countLeft;
	}
	if ((r - l + 1) <= parallelThreshold)
	{
		countLeft = new size_t[numberOfBins];
		for (size_t j = 0; j < numberOfBins; j++)
			countLeft[j] = 0;

		for (size_t current = l; current <= r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
			countLeft[(inArray[current] >> shiftRight) & 0xff]++;

		return countLeft;
	}

	size_t m = (r + l) / 2;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	Concurrency::parallel_invoke(
#else
	tbb::parallel_invoke(
#endif
		[&] { countLeft  = HistogramOneByteComponentParallel <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l,     m, shiftRight, parallelThreshold); },
		[&] { countRight = HistogramOneByteComponentParallel <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, m + 1, r, shiftRight, parallelThreshold); }
	);
	// Combine left and right results
	for (size_t j = 0; j < numberOfBins; j++)
		countLeft[j] += countRight[j];

	delete[] countRight;

	return countLeft;
}


// Simplified the implementation of the inner loop.
template< class _Type, unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, long Threshold >
inline void _RadixSort_Unsigned_PowerOf2Radix_Par_L1(_Type* a, size_t a_size, _Type bitMask, unsigned long shiftRightAmount)
{
	size_t last = a_size - 1;
#if 0
	size_t count[PowerOfTwoRadix];

	for (unsigned long i = 0; i < PowerOfTwoRadix; i++)     count[i] = 0;
	for (size_t _current = 0; _current <= last; _current++)	    // Scan the array and count the number of times each value appears
		count[(unsigned long)((a[_current] & bitMask) >> shiftRightAmount)]++;
#else
	size_t* count = HistogramOneByteComponentParallel< PowerOfTwoRadix, Log2ofPowerOfTwoRadix >(a, 0, last, shiftRightAmount);
#endif

	size_t startOfBin[PowerOfTwoRadix + 1], endOfBin[PowerOfTwoRadix], nextBin = 1;
	startOfBin[0] = endOfBin[0] = 0;    startOfBin[PowerOfTwoRadix] = 0;			// sentinal
	for (unsigned long i = 1; i < PowerOfTwoRadix; i++)
		startOfBin[i] = endOfBin[i] = startOfBin[i - 1] + count[i - 1];

	for (size_t _current = 0; _current <= last; )
	{
		unsigned long digit;
		_Type _current_element = a[_current];	// get the compiler to recognize that a register can be used for the loop instead of a[_current] memory location
		while (endOfBin[digit = (unsigned long)((_current_element & bitMask) >> shiftRightAmount)] != _current)  _swap(_current_element, a[endOfBin[digit]++]);
		a[_current] = _current_element;

		endOfBin[digit]++;
		while (endOfBin[nextBin - 1] == startOfBin[nextBin])  nextBin++;	// skip over empty and full bins, when the end of the current bin reaches the start of the next bin
		_current = endOfBin[nextBin - 1];
	}
	bitMask >>= Log2ofPowerOfTwoRadix;
	if (bitMask != 0)						// end recursion when all the bits have been processes
	{
		if (shiftRightAmount >= Log2ofPowerOfTwoRadix)	shiftRightAmount -= Log2ofPowerOfTwoRadix;
		else											shiftRightAmount = 0;

#if 0
		for (unsigned long i = 0; i < PowerOfTwoRadix; i++)
		{
			size_t numberOfElements = endOfBin[i] - startOfBin[i];		// endOfBin actually points to one beyond the bin
			if (numberOfElements >= Threshold)
				_RadixSort_Unsigned_PowerOf2Radix_Par_L1< _Type, PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(&a[startOfBin[i]], numberOfElements, bitMask, shiftRightAmount);
			else if (numberOfElements >= 2)
				insertionSortSimilarToSTLnoSelfAssignment(&a[startOfBin[i]], numberOfElements);
		}
#else
		// Multi-core version of the algorithm
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		Concurrency::task_group g;
#else
		tbb::task_group g;
#endif
		for (unsigned long i = 0; i < PowerOfTwoRadix; i++)
		{
			size_t numberOfElements = endOfBin[i] - startOfBin[i];
			if (numberOfElements >= Threshold)		// endOfBin actually points to one beyond the bin
				g.run([=] {							// important to not pass by reference, as all tasks will then get the same/last value
					_RadixSort_Unsigned_PowerOf2Radix_Par_L1< _Type, PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(&a[startOfBin[i]], numberOfElements, bitMask, shiftRightAmount);
				});
			else if (numberOfElements >= 2)
				insertionSortSimilarToSTLnoSelfAssignment(&a[startOfBin[i]], numberOfElements);
		}
		g.wait();	// TODO: Change this to not wait, as it is not necessary to wait for all tasks to complete
#endif
	}
}

inline void HybridSortPar(unsigned long* a, unsigned long a_size)
{
	if (a_size < 2)	return;

	const long PowerOfTwoRadix = 256;
	const long Log2ofPowerOfTwoRadix = 8;
	const long Threshold = 100;

	unsigned long bitMask = 0x80000000;	// bitMask controls how many bits we process at a time
	unsigned long shiftRightAmount = 31;

	for (unsigned long i = 2; i < PowerOfTwoRadix; )	// if not power-of-two value then it will do up to the largest power-of-two value
	{													// that's smaller than the value provided (e.g. radix-10 will do radix-8)
		bitMask |= (bitMask >> 1);
		shiftRightAmount -= 1;
		i <<= 1;
	}

	if (a_size >= Threshold)
		_RadixSort_Unsigned_PowerOf2Radix_Par_L1< unsigned long, PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(a, a_size, bitMask, shiftRightAmount);
	else
		insertionSortSimilarToSTLnoSelfAssignment(a, a_size);
		//insertionSortHybrid(a, a_size);
}

#endif