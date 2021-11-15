// TODO: Allocate a single array (cache-line aligned) for all the count arrays and index into it for each of the counts
// TODO: Switch histogram calculation from mask/shift to union
// TODO: Create a version of Radix Sort that handles 64-bit indexes (size_t) for arrays larger than 4GigaElements
// TODO: Detect the size of array and use unsigned/32-bit counts for smaller arrays and size_t/64-bit counts for larger arrays
#pragma once

#ifndef _RadixSortLSD_h
#define _RadixSortLSD_h

#include "RadixSortCommon.h"
#include "InsertionSort.h"

template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline unsigned long** HistogramByteComponents(unsigned long inArray[], int l, int r)
{
	const unsigned numberOfDigits = Log2ofPowerOfTwoRadix;
	const unsigned numberOfBins = PowerOfTwoRadix;

	unsigned long** count = new unsigned long* [numberOfDigits];

	for (unsigned i = 0; i < numberOfDigits; i++)
	{
		count[i] = new unsigned long[numberOfBins];
		for (unsigned j = 0; j < numberOfBins; j++)
			count[i][j] = 0;
	}

	// Faster version, since it doesn't use a 2-D array, reducing one level of indirection
	unsigned long* count0 = count[0];
	unsigned long* count1 = count[1];
	unsigned long* count2 = count[2];
	unsigned long* count3 = count[3];

	for (int current = l; current <= r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
	{
		unsigned long value = inArray[current];
		count0[ value        & 0xff]++;
		count1[(value >>  8) & 0xff]++;
		count2[(value >> 16) & 0xff]++;
		count3[(value >> 24) & 0xff]++;
	}
	return count;
}

template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline unsigned long* HistogramByteComponents_1(unsigned long inArray[], size_t l, size_t r)
{
	const unsigned numberOfDigits = Log2ofPowerOfTwoRadix;
	const unsigned numberOfBins   = PowerOfTwoRadix;
	
	unsigned long* count = new unsigned long [numberOfDigits * numberOfBins];

	for (unsigned i = 0; i < numberOfDigits * numberOfBins; i++)
		count[i] = 0;

	unsigned long* count0 = count + (0 * numberOfBins);
	unsigned long* count1 = count + (1 * numberOfBins);
	unsigned long* count2 = count + (2 * numberOfBins);
	unsigned long* count3 = count + (3 * numberOfBins);

	for (size_t current = l; current <= r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
	{
		unsigned long value = inArray[current];
		count0[ value        & 0xff]++;
		count1[(value >>  8) & 0xff]++;
		count2[(value >> 16) & 0xff]++;
		count3[(value >> 24) & 0xff]++;
	}
	return count;
}

// Serial LSD Radix Sort, with Counting separated into its own phase, followed by a permutation phase, as is done in HPCsharp in C#
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, long Threshold>
inline void _RadixSortLSD_StableUnsigned_PowerOf2RadixScalar_TwoPhase(unsigned long* input_array, unsigned long* output_array, long last, unsigned long bitMask, unsigned long shiftRightAmount, bool inputArrayIsDestination)
{
	const unsigned long numberOfBins = PowerOfTwoRadix;
	unsigned long* _input_array = input_array;
	unsigned long* _output_array = output_array;
	bool _output_array_has_result = false;
	unsigned long currentDigit = 0;

	unsigned long** count2D = HistogramByteComponents <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(input_array, 0, last);

	while (bitMask != 0)						// end processing digits when all the mask bits have been processes and shift out, leaving none
	{
		unsigned long* count = count2D[currentDigit];

		long startOfBin[numberOfBins];
		//long endOfBin[numberOfBins];
		alignas(64) long endOfBin[numberOfBins];
		//printf("endOfBin address = %p\n", endOfBin);
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

	const unsigned long numberOfDigits = Log2ofPowerOfTwoRadix;	// deallocate 2D count array, which was allocated in Histogram
	for (unsigned i = 0; i < numberOfDigits; i++)
		delete[] count2D[i];
	delete[] count2D;
}

// Serial LSD Radix Sort, with Counting separated into its own phase, followed by a permutation phase, as is done in HPCsharp in C#
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, long Threshold>
inline void _RadixSortLSD_StableUnsigned_PowerOf2RadixScalar_TwoPhase_1(unsigned long* input_array, unsigned long* output_array, size_t last, unsigned long bitMask, unsigned long shiftRightAmount, bool inputArrayIsDestination)
{
	const unsigned long numberOfBins = PowerOfTwoRadix;
	unsigned long* _input_array = input_array;
	unsigned long* _output_array = output_array;
	bool _output_array_has_result = false;
	unsigned long currentDigit = 0;
	unsigned long maxDigit = sizeof(unsigned long);
	const unsigned long BitMask = 0xff;

	unsigned long* count2D = HistogramByteComponents_1 <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(input_array, 0, last);

	while (currentDigit <= maxDigit)						// end processing digits when all the mask bits have been processes and shift out, leaving none
	{
		unsigned long* count = count2D + (currentDigit * numberOfBins);

		alignas(64) unsigned long endOfBin[numberOfBins];
		//printf("endOfBin address = %p\n", endOfBin);
		endOfBin[0] = 0;
		for (unsigned long i = 1; i < numberOfBins; i++)
			endOfBin[i] = endOfBin[i - 1] + count[i - 1];

		// permutation phase
		for (size_t _current = 0; _current <= last; _current++)
			_output_array[endOfBin[(_input_array[_current] >> shiftRightAmount) & BitMask]++] = _input_array[_current];

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

	delete[] count2D;
}

// LSD Radix Sort - stable (LSD has to be, and this may preclude LSD Radix from being able to be in-place)
inline void RadixSortLSDPowerOf2Radix_unsigned_TwoPhase(unsigned long* a, unsigned long* b, size_t a_size)
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
		_RadixSortLSD_StableUnsigned_PowerOf2RadixScalar_TwoPhase_1< PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(a, b, a_size - 1, bitMask, shiftRightAmount, false);
	}
	else {
		// TODO: Substitute Merge Sort, as it will get rid off the for loop, since it's internal to MergeSort
		insertionSortSimilarToSTLnoSelfAssignment(a, a_size);
		for (unsigned long j = 0; j < a_size; j++)	// copy from input array to the destination array
			b[j] = a[j];
	}
}

// Permute phase of LSD Radix Sort with de-randomized write memory accesses
// Derandomizes system memory accesses by buffering all Radix bin accesses, turning 256-bin random memory writes into sequential writes
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, long Threshold, unsigned long BufferDepth>
inline void _RadixSortLSD_StableUnsigned_PowerOf2Radix_PermuteDerandomized(unsigned long* input_array, unsigned long* output_array, long startIndex, long endIndex, unsigned long bitMask, unsigned long shiftRightAmount,
	long* endOfBin, unsigned long bufferIndex[], unsigned long bufferDerandomize[][BufferDepth])
{
	const unsigned long numberOfBins = PowerOfTwoRadix;

	for (long _current = startIndex; _current <= endIndex; _current++)
	{
		unsigned long digit = extractDigit(input_array[_current], bitMask, shiftRightAmount);
		if (bufferIndex[digit] < BufferDepth)
		{
			bufferDerandomize[digit][bufferIndex[digit]++] = input_array[_current];
		}
		else
		{
			unsigned long outIndex = endOfBin[digit];
			unsigned long* buff = &(bufferDerandomize[digit][0]);
#if 1
			memcpy(&(output_array[outIndex]), buff, BufferDepth * sizeof(unsigned long));	// significantly faster than a for loop
#else
			unsigned long* outBuff = &(output_array[outIndex]);
			for (unsigned long i = 0; i < BufferDepth; i++)
				*outBuff++ = *buff++;
#endif
			endOfBin[digit] += BufferDepth;
			bufferDerandomize[digit][0] = input_array[_current];
			bufferIndex[digit] = 1;
		}
	}
	// Flush all the derandomization buffers
	for (unsigned long whichBuff = 0; whichBuff < numberOfBins; whichBuff++)
	{
		unsigned long numOfElementsInBuff = bufferIndex[whichBuff];
		for (unsigned long i = 0; i < numOfElementsInBuff; i++)
			output_array[endOfBin[whichBuff]++] = bufferDerandomize[whichBuff][i];
		bufferIndex[whichBuff] = 0;
	}
}

// Derandomizes system memory accesses by buffering all Radix bin accesses, turning 256-bin random memory writes into sequential writes
// Parallel LSD Radix Sort, with Counting separated into its own parallel phase, followed by a serial permutation phase, as is done in HPCsharp in C#
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, long Threshold>
void _RadixSortLSD_StableUnsigned_PowerOf2Radix_TwoPhase_DeRandomize(unsigned long* input_array, unsigned long* output_array, long last, unsigned long bitMask, unsigned long shiftRightAmount, bool inputArrayIsDestination)
{
	const unsigned long numberOfBins = PowerOfTwoRadix;
	unsigned long* _input_array = input_array;
	unsigned long* _output_array = output_array;
	bool _output_array_has_result = false;
	unsigned long currentDigit = 0;
	static const unsigned long bufferDepth = 16;
	__declspec(align(64)) unsigned long bufferDerandomize[numberOfBins][bufferDepth];
	__declspec(align(64)) unsigned long bufferIndex[numberOfBins] = { 0 };

	unsigned long* count2D = HistogramByteComponents_1 <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(input_array, 0, last);

	while (bitMask != 0)						// end processing digits when all the mask bits have been processes and shift out, leaving none
	{
		unsigned long* count = count2D + (currentDigit * numberOfBins);

		long startOfBin[numberOfBins], endOfBin[numberOfBins];
		startOfBin[0] = endOfBin[0] = 0;
		for (unsigned long i = 1; i < numberOfBins; i++)
			startOfBin[i] = endOfBin[i] = startOfBin[i - 1] + count[i - 1];

		_RadixSortLSD_StableUnsigned_PowerOf2Radix_PermuteDerandomized< PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold, bufferDepth>(
			_input_array, _output_array, 0, last, bitMask, shiftRightAmount, endOfBin, bufferIndex, bufferDerandomize);

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
inline void RadixSortLSDPowerOf2Radix_unsigned_TwoPhase_DeRandomize(unsigned long* a, unsigned long* b, unsigned long a_size)
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
		_RadixSortLSD_StableUnsigned_PowerOf2Radix_TwoPhase_DeRandomize< PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(a, b, a_size - 1, bitMask, shiftRightAmount, false);
	}
	else {
		// TODO: Substitute Merge Sort, as it will get rid off the for loop, since it's internal to MergeSort
		insertionSortSimilarToSTLnoSelfAssignment(a, a_size);
		for (unsigned long j = 0; j < a_size; j++)	// copy from input array to the destination array
			b[j] = a[j];
	}
}


#endif