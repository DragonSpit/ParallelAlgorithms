#pragma once

#ifndef _RadixSortMsdParallel_h
#define _RadixSortMsdParallel_h

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
#include "RadixSortMSD.h"
#include "InsertionSort.h"

// This version did not seem to speed up over the single count array version. It proves that Histogram is not the bottleneck.
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline size_t* HistogramOneByteComponentParallel_2(unsigned long inArray[], size_t l, size_t r, unsigned long shiftRight, size_t parallelThreshold = 64 * 1024)
{
	const unsigned long numberOfBins = PowerOfTwoRadix;

	size_t* countLeft_0 = NULL;
	size_t* countLeft_1 = NULL;
	size_t* countLeft_2 = NULL;
	size_t* countLeft_3 = NULL;
	size_t* countRight  = NULL;

	if (l > r)      // zero elements to compare
	{
		countLeft_0 = new size_t[numberOfBins]{};
		return countLeft_0;
	}
	if ((r - l + 1) <= parallelThreshold)
	{
		countLeft_0 = new size_t[numberOfBins]{};
		countLeft_1 = new size_t[numberOfBins]{};
		countLeft_2 = new size_t[numberOfBins]{};
		countLeft_3 = new size_t[numberOfBins]{};

		size_t last_by_four = l + ((r - l) / 4) * 4;
		size_t current = l;
		for (; current < last_by_four;)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
		{
			countLeft_0[(inArray[current] >> shiftRight) & 0xff]++;  current++;
			countLeft_1[(inArray[current] >> shiftRight) & 0xff]++;  current++;
			countLeft_2[(inArray[current] >> shiftRight) & 0xff]++;  current++;
			countLeft_3[(inArray[current] >> shiftRight) & 0xff]++;  current++;
		}
		for (; current < r; current++)    // possibly last element
			countLeft_0[(inArray[current] >> shiftRight) & 0xff]++;

		// Combine the two count arrays into a single arrray to return
		for (size_t count_index = 0; count_index < numberOfBins; count_index++)
		{
			countLeft_0[count_index] += countLeft_1[count_index];
			countLeft_0[count_index] += countLeft_2[count_index];
			countLeft_0[count_index] += countLeft_3[count_index];
		}

		delete[] countLeft_3;
		delete[] countLeft_2;
		delete[] countLeft_1;
		return countLeft_0;
	}

	size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	Concurrency::parallel_invoke(
#else
	tbb::parallel_invoke(
#endif
		[&] { countLeft_0 = HistogramOneByteComponentParallel_2 <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l,     m, shiftRight, parallelThreshold); },
		[&] { countRight  = HistogramOneByteComponentParallel_2 <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, m + 1, r, shiftRight, parallelThreshold); }
	);
	// Combine left and right results
	for (size_t j = 0; j < numberOfBins; j++)
		countLeft_0[j] += countRight[j];

	delete[] countRight;

	return countLeft_0;
}

template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline size_t* HistogramOneByteComponentParallel(unsigned long inArray[], size_t l, size_t r, unsigned long shiftRight, size_t parallelThreshold = 64 * 1024)
{
	const unsigned long numberOfBins = PowerOfTwoRadix;

	size_t* countLeft  = NULL;
	size_t* countRight = NULL;

	if (l > r)      // zero elements to compare
	{
		countLeft = new size_t[numberOfBins]{};
		return countLeft;
	}
	if ((r - l + 1) <= parallelThreshold)
	{
		countLeft = new size_t[numberOfBins]{};

		for (size_t current = l; current <= r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
			countLeft[(inArray[current] >> shiftRight) & 0xff]++;

		return countLeft;
	}

	size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;

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

#if 1
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

// Permute phase of MSD Radix Sort with de-randomized write memory accesses
// Derandomizes system memory accesses by buffering all Radix bin accesses, turning 256-bin random memory writes into sequential writes
// Separates read pointers from write pointers of/to each bin
// Idea: It may be better to implement read/write buffering where all of the swaps happen within those buffers with writes dumping out to the bins and fetching the next buffer
//       It's similar to caching, but doing it in a more cache-friendly way where all of the bin-buffers can fit into the cache and not map on top of each other. Otherwise, with
//       bins we get data-dependent cache thrashing. Plus, all of the swapping will be within the buffers which are well organized for caching.
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, long Threshold, unsigned long BufferDepth>
inline void _RadixSortMSD_StableUnsigned_PowerOf2Radix_PermuteDerandomized_1(unsigned long* inout_array, size_t startIndex, size_t endIndex, unsigned long bitMask, unsigned long shiftRightAmount,
	size_t* startOfBin, size_t* endOfBin, unsigned long bufferIndex[], unsigned long bufferDerandomize[][BufferDepth])
{
	// TODO: This version is broken and needs to be fixed!!

	//printf("Permute Derandomized #1: startIndex = %zu   endIndex = %zu   bitMask = %lx   shiftRight = %lu \n", startIndex, endIndex, bitMask, shiftRightAmount);
	const unsigned long numberOfBins = PowerOfTwoRadix;
	size_t writeEndOfBin[numberOfBins];									// write pointers to each bin
	std::copy(endOfBin + 0, endOfBin + numberOfBins, writeEndOfBin);	// copy read pointers (endOfBin) to write pointers

	size_t nextBin = 1;
	for (size_t _current = startIndex; _current <= endIndex;)
	{
		unsigned long digit;
		unsigned long _current_element = inout_array[_current];	// get the compiler to recognize that a register can be used for the loop instead of a[_current] memory location
		while (endOfBin[digit = (unsigned long)((_current_element & bitMask) >> shiftRightAmount)] != _current)
		{
			unsigned long tmp = _current_element;				// read of the current element to squirl it away
			_current_element = inout_array[endOfBin[digit]];	// read from a bin and place in the current element
			endOfBin[digit]++;									// advance the read pointer of that bin

			//a[endOfBin[digit]] = tmp;							// write the current element to that bin. This is the one (the write) which needs to be buffered

			if (bufferIndex[digit] < BufferDepth)
			{
				bufferDerandomize[digit][bufferIndex[digit]++] = tmp;	// write the current element into the buffer for that bin
			}
			else
			{
				unsigned long outIndex = writeEndOfBin[digit];
				unsigned long* buff = &(bufferDerandomize[digit][0]);
#if 1
				memcpy(&(inout_array[outIndex]), buff, BufferDepth * sizeof(unsigned long));	// significantly faster than a for loop. Need to try std::copy - simpler interface
#else
				unsigned long* outBuff = &(output_array[outIndex]);
				for (unsigned long i = 0; i < BufferDepth; i++)
					*outBuff++ = *buff++;
#endif
				writeEndOfBin[digit] += BufferDepth;
				bufferDerandomize[digit][0] = tmp;						// write the current element into the buffer for that bin
				bufferIndex[digit] = 1;
			}
		}
		inout_array[_current] = _current_element;	// write the current element into the current bin
		endOfBin[digit]++;							// advance the read  pointer
		writeEndOfBin[digit]++;						// advance the write pointer

		while (endOfBin[nextBin - 1] == startOfBin[nextBin])  nextBin++;	// skip over empty and full bins, when the end of the current bin reaches the start of the next bin
		_current = endOfBin[nextBin - 1];
	}
	// Flush all the derandomization buffers
	for (unsigned long whichBuff = 0; whichBuff < numberOfBins; whichBuff++)
	{
		unsigned long numOfElementsInBuff = bufferIndex[whichBuff];
		for (size_t i = 0; i < numOfElementsInBuff; i++)
			inout_array[writeEndOfBin[whichBuff]++] = bufferDerandomize[whichBuff][i];
		bufferIndex[whichBuff] = 0;
	}
}

// Permute phase of MSD Radix Sort with de-randomized write memory accesses
// Derandomizes system memory accesses by buffering all Radix bin accesses, turning 256-bin random memory writes into sequential writes
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, long Threshold, unsigned long BufferDepth>
inline void _RadixSortMSD_StableUnsigned_PowerOf2Radix_PermuteDerandomized(unsigned long* inout_array, size_t startIndex, size_t endIndex, unsigned long bitMask, unsigned long shiftRightAmount,
	size_t* endOfBin, unsigned long bufferIndex[], unsigned long bufferDerandomize[][BufferDepth])
{
	printf("Permute Derandomized: startIndex = %zu   endIndex = %zu   bitMask = %lx   shiftRight = %lu \n", startIndex, endIndex, bitMask, shiftRightAmount);
	const unsigned long numberOfBins = PowerOfTwoRadix;

	for (size_t _current = startIndex; _current <= endIndex; _current++)
	{
		unsigned long digit = extractDigit(inout_array[_current], bitMask, shiftRightAmount);
		if (bufferIndex[digit] < BufferDepth)
		{
			bufferDerandomize[digit][bufferIndex[digit]++] = inout_array[_current];
		}
		else
		{
			unsigned long outIndex = endOfBin[digit];
			unsigned long* buff = &(bufferDerandomize[digit][0]);
#if 1
			memcpy(&(inout_array[outIndex]), buff, BufferDepth * sizeof(unsigned long));	// significantly faster than a for loop
#else
			unsigned long* outBuff = &(output_array[outIndex]);
			for (unsigned long i = 0; i < BufferDepth; i++)
				*outBuff++ = *buff++;
#endif
			endOfBin[digit] += BufferDepth;
			bufferDerandomize[digit][0] = inout_array[_current];
			bufferIndex[digit] = 1;
		}
	}
	// Flush all the derandomization buffers
	for (unsigned long whichBuff = 0; whichBuff < numberOfBins; whichBuff++)
	{
		unsigned long numOfElementsInBuff = bufferIndex[whichBuff];
		for (size_t i = 0; i < numOfElementsInBuff; i++)
			inout_array[endOfBin[whichBuff]++] = bufferDerandomize[whichBuff][i];
		bufferIndex[whichBuff] = 0;
	}
}

// Simplified the implementation of the inner loop.
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, long Threshold >
inline void _RadixSort_Unsigned_PowerOf2Radix_Derandomized_Par_L1(unsigned long* a, size_t a_size, unsigned long bitMask, unsigned long shiftRightAmount)
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

	size_t startOfBin[PowerOfTwoRadix + 1], endOfBin[PowerOfTwoRadix];
	startOfBin[0] = endOfBin[0] = 0;    startOfBin[PowerOfTwoRadix] = 0;			// sentinal
	for (unsigned long i = 1; i < PowerOfTwoRadix; i++)
		startOfBin[i] = endOfBin[i] = startOfBin[i - 1] + count[i - 1];

#if 1
	size_t nextBin = 1;
	for (size_t _current = 0; _current <= last; )
	{
		unsigned long digit;
		unsigned long _current_element = a[_current];	// get the compiler to recognize that a register can be used for the loop instead of a[_current] memory location
		while (endOfBin[digit = (unsigned long)((_current_element & bitMask) >> shiftRightAmount)] != _current)  _swap(_current_element, a[endOfBin[digit]++]);
		a[_current] = _current_element;

		endOfBin[digit]++;
		while (endOfBin[nextBin - 1] == startOfBin[nextBin])  nextBin++;	// skip over empty and full bins, when the end of the current bin reaches the start of the next bin
		_current = endOfBin[nextBin - 1];
	}
#else
	// TODO: This version is broken and needs to be fixed!!
	const unsigned long numberOfBins = PowerOfTwoRadix;
	static const unsigned long bufferDepth = 128;
	__declspec(align(64)) unsigned long bufferDerandomize[numberOfBins][bufferDepth];
	__declspec(align(64)) unsigned long bufferIndex[numberOfBins] = { 0 };

	//printf("Before Permute Derandomized \n");
	_RadixSortMSD_StableUnsigned_PowerOf2Radix_PermuteDerandomized_1< PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold, bufferDepth>(
		a, 0, last, bitMask, shiftRightAmount, startOfBin, endOfBin, bufferIndex, bufferDerandomize);
	//printf("After Permute Derandomized \n");
#endif
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
				_RadixSort_Unsigned_PowerOf2Radix_Derandomized_Par_L1< PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(&a[startOfBin[i]], numberOfElements, bitMask, shiftRightAmount);
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
				_RadixSort_Unsigned_PowerOf2Radix_Derandomized_Par_L1< PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(&a[startOfBin[i]], numberOfElements, bitMask, shiftRightAmount);
					});
			else if (numberOfElements >= 2)
				insertionSortSimilarToSTLnoSelfAssignment(&a[startOfBin[i]], numberOfElements);
		}
		g.wait();	// TODO: Change this to not wait, as it is not necessary to wait for all tasks to complete
#endif
	}
}

inline void parallel_hybrid_inplace_msd_radix_sort(unsigned long* a, size_t a_size)
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
	{
		_RadixSort_Unsigned_PowerOf2Radix_Par_L1< unsigned long, PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(a, a_size, bitMask, shiftRightAmount);	// same speed as de-randomization on 6-core
		//_RadixSort_Unsigned_PowerOf2Radix_Derandomized_Par_L1< PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(a, a_size, bitMask, shiftRightAmount);
	}
	else
		insertionSortSimilarToSTLnoSelfAssignment(a, a_size);
		//insertionSortHybrid(a, a_size);
}

#endif