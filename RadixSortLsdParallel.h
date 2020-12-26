#pragma once

#include "InsertionSort.h"
#include "BinarySearch.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <ppl.h>
#else
#include <stddef.h>
#include <stdio.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <ratio>
#include <vector>
#include <execution>
#include <thread>
#endif

template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline unsigned long** HistogramByteComponentsParallel(unsigned long inArray[], int l, int r, int parallelThreshold = 16 * 1024)
{
	const unsigned long numberOfDigits = Log2ofPowerOfTwoRadix;
	const unsigned long numberOfBins = PowerOfTwoRadix;

	unsigned long** countLeft = new unsigned long* [numberOfDigits];

	for (int i = 0; i < numberOfDigits; i++)
	{
		countLeft[i] = new unsigned long[numberOfBins];
		for (int j = 0; j < numberOfBins; j++)
			countLeft[i][j] = 0;
	}

	if (l > r)      // zero elements to compare
		return countLeft;
	if ((r - l + 1) <= parallelThreshold)
	{
		// Faster version, since it doesn't use a 2-D array, reducing one level of indirection
		unsigned long* count0 = countLeft[0];
		unsigned long* count1 = countLeft[1];
		unsigned long* count2 = countLeft[2];
		unsigned long* count3 = countLeft[3];
#if 1
		for (int current = l; current <= r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
		{
			unsigned long value = inArray[current];
			count0[value & 0xff]++;
			count1[(value >> 8) & 0xff]++;
			count2[(value >> 16) & 0xff]++;
			count3[(value >> 24) & 0xff]++;
		}
#else
		// Seems to be about the same performance as masking and shifting
		for (int current = l; current <= r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
		{
			either value;
			value.dw = inArray[current];
			count0[value.bytes.b[0]]++;
			count1[value.bytes.b[1]]++;
			count2[value.bytes.b[2]]++;
			count3[value.bytes.b[3]]++;
		}
#endif
		return countLeft;
	}

	int m = ((r + l) / 2);

	unsigned long** countRight = new unsigned long* [numberOfDigits];

	for (int i = 0; i < numberOfDigits; i++)
	{
		countRight[i] = new unsigned long[numberOfBins];
		for (int j = 0; j < numberOfBins; j++)
			countRight[i][j] = 0;
	}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	Concurrency::parallel_invoke(
#else
	tbb::parallel_invoke(
#endif
		[&] { countLeft  = HistogramByteComponentsParallel <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l,     m, parallelThreshold); },
		[&] { countRight = HistogramByteComponentsParallel <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, m + 1, r, parallelThreshold); }
	);
	// Combine left and right results
	for (int i = 0; i < numberOfDigits; i++)
		for (int j = 0; j < numberOfBins; j++)
			countLeft[i][j] += countRight[i][j];

	return countLeft;
}

// Serial LSD Radix Sort, with Counting separated into its own phase, followed by a permutation phase, as is done in HPCsharp in C#
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, long Threshold>
inline void _RadixSortLSD_StableUnsigned_PowerOf2RadixParallel_TwoPhase(unsigned long* input_array, unsigned long* output_array, long last, unsigned long bitMask, unsigned long shiftRightAmount, bool inputArrayIsDestination)
{
	const unsigned long numberOfBins = PowerOfTwoRadix;
	unsigned long* _input_array = input_array;
	unsigned long* _output_array = output_array;
	bool _output_array_has_result = false;
	unsigned long currentDigit = 0;

	unsigned long** count2D = HistogramByteComponentsParallel <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(input_array, 0, last);

	while (bitMask != 0)						// end processing digits when all the mask bits have been processes and shift out, leaving none
	{
		unsigned long* count = count2D[currentDigit];

		long startOfBin[numberOfBins], endOfBin[numberOfBins];
		startOfBin[0] = endOfBin[0] = 0;
		for (unsigned long i = 1; i < numberOfBins; i++)
			startOfBin[i] = endOfBin[i] = startOfBin[i - 1] + count[i - 1];

		for (long _current = 0; _current <= last; _current++)	// permutation phase
			_output_array[endOfBin[extractDigit(_input_array[_current], bitMask, shiftRightAmount)]++] = _input_array[_current];

		bitMask <<= Log2ofPowerOfTwoRadix;
		shiftRightAmount += Log2ofPowerOfTwoRadix;
		_output_array_has_result = !_output_array_has_result;
		std::swap(_input_array, _output_array);
		currentDigit++;
	}
	// Done with processing, copy all of the bins
	if (_output_array_has_result && inputArrayIsDestination)
		for (long _current = 0; _current <= last; _current++)	// copy from output array into the input array
			_input_array[_current] = _output_array[_current];
	if (!_output_array_has_result && !inputArrayIsDestination)
		for (long _current = 0; _current <= last; _current++)	// copy from input array back into the output array
			_output_array[_current] = _input_array[_current];
}

// LSD Radix Sort - stable (LSD has to be, and this may preclude LSD Radix from being able to be in-place)
inline void RadixSortLSDPowerOf2RadixParallel_unsigned_TwoPhase(unsigned long* a, unsigned long* b, unsigned long a_size)
{
	const unsigned long Threshold = 100;	// Threshold of when to switch to using Insertion Sort
	const unsigned long PowerOfTwoRadix = 256;
	const unsigned long Log2ofPowerOfTwoRadix = 8;
	// Create bit-mask and shift right amount
	unsigned long shiftRightAmount = 0;
	unsigned long bitMask = (unsigned long)(((unsigned long)(PowerOfTwoRadix - 1)) << shiftRightAmount);	// bitMask controls/selects how many and which bits we process at a time

	// The beauty of using template arguments instead of function parameters for the Threshold and Log2ofPowerOfTwoRadix is
	// they are not pushed on the stack and are treated as constants, but local.
	if (a_size >= Threshold) {
		_RadixSortLSD_StableUnsigned_PowerOf2RadixParallel_TwoPhase< PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(a, b, a_size - 1, bitMask, shiftRightAmount, false);
	}
	else {
		// TODO: Substitute Merge Sort, as it will get rid off the for loop, since it's internal to MergeSort
		insertionSortSimilarToSTLnoSelfAssignment(a, a_size);
		for (unsigned long j = 0; j < a_size; j++)	// copy from input array to the destination array
			b[j] = a[j];
	}
}
