#pragma once

#ifndef _RadixSortMsdParallel_h
#define _RadixSortMsdParallel_h

// TBB-only implementation
#include "tbb/tbb.h"
#include <tbb/parallel_invoke.h>

#include "InsertionSort.h"
#include "BinarySearch.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <ratio>
#include <vector>
#include <execution>
#include <thread>

using namespace tbb;

#include "RadixSortCommon.h"
#include "RadixSortMSD.h"
#include "InsertionSort.h"
#include "HistogramParallel.h"

namespace ParallelAlgorithms
{
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
			unsigned digit;
			_Type _current_element = a[_current];	// get the compiler to recognize that a register can be used for the loop instead of a[_current] memory location
			while (endOfBin[digit = (unsigned)((_current_element & bitMask) >> shiftRightAmount)] != _current)  _swap(_current_element, a[endOfBin[digit]++]);
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
#if defined(USE_PPL)
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
		const unsigned long NumberOfBins = PowerOfTwoRadix;
		size_t writeEndOfBin[NumberOfBins];									// write pointers to each bin
		std::copy(endOfBin + 0, endOfBin + NumberOfBins, writeEndOfBin);	// copy read pointers (endOfBin) to write pointers

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
		for (unsigned long whichBuff = 0; whichBuff < NumberOfBins; whichBuff++)
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
		const unsigned long NumberOfBins = PowerOfTwoRadix;

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
		for (unsigned long whichBuff = 0; whichBuff < NumberOfBins; whichBuff++)
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
		const unsigned long NumberOfBins = PowerOfTwoRadix;
		static const unsigned long bufferDepth = 128;
		__declspec(align(64)) unsigned long bufferDerandomize[NumberOfBins][bufferDepth];
		__declspec(align(64)) unsigned long bufferIndex[NumberOfBins] = { 0 };

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
#if defined(USE_PPL)
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

	inline void parallel_hybrid_inplace_msd_radix_sort(unsigned* a, size_t a_size)
	{
		if (a_size < 2)	return;

		const long PowerOfTwoRadix = 256;
		const long Log2ofPowerOfTwoRadix = 8;
		const long Threshold = 100;

		unsigned bitMask = 0x80000000;	// bitMask controls how many bits we process at a time
		unsigned shiftRightAmount = 31;

		for (size_t i = 2; i < PowerOfTwoRadix; )	// if not power-of-two value then it will do up to the largest power-of-two value
		{													// that's smaller than the value provided (e.g. radix-10 will do radix-8)
			bitMask |= (bitMask >> 1);
			shiftRightAmount -= 1;
			i <<= 1;
		}

		if (a_size >= Threshold)
		{
			_RadixSort_Unsigned_PowerOf2Radix_Par_L1< unsigned, PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(a, a_size, bitMask, shiftRightAmount);	// same speed as de-randomization on 6-core
			//_RadixSort_Unsigned_PowerOf2Radix_Derandomized_Par_L1< PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(a, a_size, bitMask, shiftRightAmount);
		}
		else
			insertionSortSimilarToSTLnoSelfAssignment(a, a_size);
		//insertionSortHybrid(a, a_size);
	}
}
#endif