// TODO: Add the same optimization for the permutation phase (improving writes for the case of constant arrays) to the derandomized version
//       as is done to the non-derandomized version. The derandomized version is currently slower on constant arrays due to the overhead of buffering
//       for the constant array case.
// TODO: Allocate a single array (cache-line aligned) for all the count arrays and index into it for each of the counts
// TODO: Create a version of Radix Sort that handles 64-bit indexes (size_t) for arrays larger than 4GigaElements
// TODO: Detect the size of array and use unsigned/32-bit counts for smaller arrays and size_t/64-bit counts for larger arrays
// TODO: sort_radix_in_place_stable_adaptive can be implemented as preventative adaptive and stable/unstable option in a single function

#ifndef _RadixSortLSD_h
#define _RadixSortLSD_h

#include <stddef.h>
#include <stdio.h>
#include <iostream>
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

#include "RadixSortCommon.h"
#include "RadixSortMSD.h"
#include "InsertionSort.h"
#include "ParallelMergeSort.h"
#include "Histogram.h"
#include "Copy.h"

extern unsigned long long physical_memory_used_in_megabytes();
extern unsigned long long physical_memory_total_in_megabytes();

namespace ExperimentalSequentialAlgorithms
{
	// Provides 20% speedup over the baseline implementation for constant arrays and does not slow random or presorted arrays down. Runs even faster with Microsoft Compiler.
	// Versus two-phase counting and permuation, this method is slower.
	// However, this technique could be used in parallel LSD Radix Sort where two-phase method is not possible and the array is already being split into chunks.
	// This algorithm does not support two-phase counting and permutation method for the same reason parallel LSD Radix Sort does not support it, as array elements move between
	// halves of the array, messing up the counts. However, it could be combined with counting while writing the data and with de-randomization of writes.
	// Splits writes into two halves to attempt to provide two independent memory writes to break any dependencies in hopes of improved pipelining.
	// Serial LSD Radix Sort, with Counting separated into its own phase, followed by a permutation phase, as is done in HPCsharp in C#
	// TODO: Test with arrays of odd size to ensure all elements are processed, as they will not be split into equal halves.
	inline void _RadixSortLSD_StableUnsigned_PowerOf2RadixScalar_1(unsigned* inout_array, unsigned* tmp_array, size_t last)
	{
		const unsigned BitsPerDigit = 8;
		const size_t NumberOfBins = (size_t)1 << BitsPerDigit;
		unsigned shiftRightAmount = 0;
		unsigned* _inout_array = inout_array;
		unsigned* _tmp_array = tmp_array;
		unsigned currentDigit = 0;
		unsigned maxDigit = sizeof(unsigned);
		const unsigned bit_mask = NumberOfBins - 1;
		size_t count_left[NumberOfBins], count_right[NumberOfBins];
		size_t right_half_start = (last + 1) / 2;

		while (currentDigit < maxDigit)						// end processing digits when all the mask bits have been processes and shift out, leaving none
		{
			HistogramByteSingleComponent(_inout_array, 0, right_half_start - 1, shiftRightAmount, count_left);
			HistogramByteSingleComponent(_inout_array, right_half_start, last, shiftRightAmount, count_right);

			//size_t* count_left  = count2D_left  + (currentDigit * NumberOfBins);  // TODO: Could move endOfBin calculation outside of this loop.
			//size_t* count_right = count2D_right + (currentDigit * NumberOfBins);
			alignas(64) size_t startOfBin_left[NumberOfBins];
			alignas(64) size_t startOfBin_right[NumberOfBins];
			//printf("endOfBin address = %p\n", endOfBin);
			startOfBin_left[0] = 0; startOfBin_right[0] = count_left[0];
			for (size_t i = 1; i < NumberOfBins; i++)
			{
				startOfBin_left[i] = startOfBin_left[i - 1] + count_left[i - 1] + count_right[i - 1];
				startOfBin_right[i] = startOfBin_left[i] + count_left[i];
			}
			//printf("startOfBin_left & startOfBin_right\n");
			//for (size_t i = 0; i < NumberOfBins; i++)
			//	printf("%zu: %zu  %zu\n", i, startOfBin_left[i], startOfBin_right[i]);
			//printf("\n");
			//const auto startTime = high_resolution_clock::now();
			// permutation phase
			// Left half of the array always has <= right half of the array number of elements.
			size_t _current_right = right_half_start;
			for (size_t _current_left = 0; _current_left < right_half_start; _current_left++, _current_right++)
			{
				_tmp_array[startOfBin_left[ (_inout_array[_current_left]  >> shiftRightAmount) & bit_mask]++] = _inout_array[_current_left];
				_tmp_array[startOfBin_right[(_inout_array[_current_right] >> shiftRightAmount) & bit_mask]++] = _inout_array[_current_right];
			}
			//if (_current_right == last)
			//	_output_array[startOfBin_right[(_input_array[_current_right] >> shiftRightAmount) & bit_mask]] = _input_array[_current_right];

			//printf("_tmp_arry:  ");
			//for (size_t i = 0; i <= last; i++)
			//	printf("%x  ", _tmp_array[i]);
			//printf("\n");

			//const auto endTime = high_resolution_clock::now();
			//print_results("Permutation: ", startTime, endTime);

			shiftRightAmount += BitsPerDigit;
			std::swap(_inout_array, _tmp_array);
			currentDigit++;
		}
	}
	// TODO: Move arrays to heap instead of stack.
	inline void _RadixSortLSD_StableUnsigned_PowerOf2RadixScalar_4X(unsigned* inout_array, unsigned* tmp_array, size_t last)
	{
		const unsigned BitsPerDigit = 8;
		const size_t NumberOfBins = (size_t)1 << BitsPerDigit;
		unsigned shiftRightAmount = 0;
		unsigned* _inout_array = inout_array;
		unsigned* _tmp_array = tmp_array;
		unsigned currentDigit = 0;
		unsigned maxDigit = sizeof(unsigned);
		const unsigned bit_mask = NumberOfBins - 1;
		size_t count_0[NumberOfBins], count_1[NumberOfBins], count_2[NumberOfBins], count_3[NumberOfBins];
		size_t quarter_1_start = (last + 1) / 4;
		size_t quarter_2_start = (last + 1) / 2;
		size_t quarter_3_start = (last + 1) * 3 / 4;

		while (currentDigit < maxDigit)						// end processing digits when all the mask bits have been processes and shift out, leaving none
		{
			HistogramByteSingleComponent(_inout_array, 0,               quarter_1_start - 1, shiftRightAmount, count_0);
			HistogramByteSingleComponent(_inout_array, quarter_1_start, quarter_2_start - 1, shiftRightAmount, count_1);
			HistogramByteSingleComponent(_inout_array, quarter_2_start, quarter_3_start - 1, shiftRightAmount, count_2);
			HistogramByteSingleComponent(_inout_array, quarter_3_start, last,                shiftRightAmount, count_3);

			alignas(64) size_t startOfBin_0[NumberOfBins];
			alignas(64) size_t startOfBin_1[NumberOfBins];
			alignas(64) size_t startOfBin_2[NumberOfBins];
			alignas(64) size_t startOfBin_3[NumberOfBins];
			//printf("endOfBin address = %p\n", endOfBin);
			startOfBin_0[0] = 0; startOfBin_1[0] = count_0[0]; startOfBin_2[0] = count_0[0] + count_1[0]; startOfBin_3[0] = count_0[0] + count_1[0] + count_2[0];
			for (size_t i = 1; i < NumberOfBins; i++)
			{
				startOfBin_0[i] = startOfBin_0[i - 1] + count_0[i - 1] + count_1[i - 1] + count_2[i - 1] + count_3[i - 1];
				startOfBin_1[i] = startOfBin_0[i] + count_0[i];
				startOfBin_2[i] = startOfBin_0[i] + count_0[i] + count_1[i];
				startOfBin_3[i] = startOfBin_0[i] + count_0[i] + count_1[i] + count_2[i];
			}
			//printf("startOfBin_left & startOfBin_right\n");
			//for (size_t i = 0; i < NumberOfBins; i++)
			//	printf("%zu: %zu  %zu\n", i, startOfBin_left[i], startOfBin_right[i]);
			//printf("\n");
			//const auto startTime = high_resolution_clock::now();
			// permutation phase
			// Left half of the array always has <= right half of the array number of elements.
			size_t _current_1 = quarter_1_start; size_t _current_2 = quarter_2_start; size_t _current_3 = quarter_3_start;
			for (size_t _current_0 = 0; _current_0 < quarter_1_start; _current_0++, _current_1++, _current_2++, _current_3++)
			{
				_tmp_array[startOfBin_0[(_inout_array[_current_0] >> shiftRightAmount) & bit_mask]++] = _inout_array[_current_0];
				_tmp_array[startOfBin_1[(_inout_array[_current_1] >> shiftRightAmount) & bit_mask]++] = _inout_array[_current_1];
				_tmp_array[startOfBin_2[(_inout_array[_current_2] >> shiftRightAmount) & bit_mask]++] = _inout_array[_current_2];
				_tmp_array[startOfBin_3[(_inout_array[_current_3] >> shiftRightAmount) & bit_mask]++] = _inout_array[_current_3];
			}
			//if (_current_right == last)
			//	_output_array[startOfBin_right[(_input_array[_current_right] >> shiftRightAmount) & bit_mask]] = _input_array[_current_right];

			//printf("_tmp_arry:  ");
			//for (size_t i = 0; i <= last; i++)
			//	printf("%x  ", _tmp_array[i]);
			//printf("\n");

			//const auto endTime = high_resolution_clock::now();
			//print_results("Permutation: ", startTime, endTime);

			shiftRightAmount += BitsPerDigit;
			std::swap(_inout_array, _tmp_array);
			currentDigit++;
		}
	}
	inline void _RadixSortLSD_StableUnsigned_PowerOf2RadixScalar_3X(unsigned* inout_array, unsigned* tmp_array, size_t last)
	{
		const unsigned BitsPerDigit = 8;
		const size_t NumberOfBins = (size_t)1 << BitsPerDigit;
		unsigned shiftRightAmount = 0;
		unsigned* _inout_array = inout_array;
		unsigned* _tmp_array = tmp_array;
		unsigned currentDigit = 0;
		unsigned maxDigit = sizeof(unsigned);
		const unsigned bit_mask = NumberOfBins - 1;
		size_t count_0[NumberOfBins], count_1[NumberOfBins], count_2[NumberOfBins];
		size_t third_1_start = (last + 1) / 3;
		size_t third_2_start = (last + 1) * 2 / 3;

		while (currentDigit < maxDigit)						// end processing digits when all the mask bits have been processes and shift out, leaving none
		{
			HistogramByteSingleComponent(_inout_array, 0, third_1_start - 1,             shiftRightAmount, count_0);
			HistogramByteSingleComponent(_inout_array, third_1_start, third_2_start - 1, shiftRightAmount, count_1);
			HistogramByteSingleComponent(_inout_array, third_2_start, last,              shiftRightAmount, count_2);

			alignas(64) size_t startOfBin_0[NumberOfBins];
			alignas(64) size_t startOfBin_1[NumberOfBins];
			alignas(64) size_t startOfBin_2[NumberOfBins];
			//printf("endOfBin address = %p\n", endOfBin);
			startOfBin_0[0] = 0; startOfBin_1[0] = count_0[0]; startOfBin_2[0] = count_0[0] + count_1[0];
			for (size_t i = 1; i < NumberOfBins; i++)
			{
				startOfBin_0[i] = startOfBin_0[i - 1] + count_0[i - 1] + count_1[i - 1] + count_2[i - 1];
				startOfBin_1[i] = startOfBin_0[i] + count_0[i];
				startOfBin_2[i] = startOfBin_0[i] + count_0[i] + count_1[i];
			}
			//printf("startOfBin_left & startOfBin_right\n");
			//for (size_t i = 0; i < NumberOfBins; i++)
			//	printf("%zu: %zu  %zu\n", i, startOfBin_left[i], startOfBin_right[i]);
			//printf("\n");
			//const auto startTime = high_resolution_clock::now();
			// permutation phase
			// Left half of the array always has <= right half of the array number of elements.
			size_t _current_1 = third_1_start; size_t _current_2 = third_2_start;
			for (size_t _current_0 = 0; _current_0 < third_1_start; _current_0++, _current_1++, _current_2++)
			{
				_tmp_array[startOfBin_0[(_inout_array[_current_0] >> shiftRightAmount) & bit_mask]++] = _inout_array[_current_0];
				_tmp_array[startOfBin_1[(_inout_array[_current_1] >> shiftRightAmount) & bit_mask]++] = _inout_array[_current_1];
				_tmp_array[startOfBin_2[(_inout_array[_current_2] >> shiftRightAmount) & bit_mask]++] = _inout_array[_current_2];
			}
			//if (_current_right == last)
			//	_output_array[startOfBin_right[(_input_array[_current_right] >> shiftRightAmount) & bit_mask]] = _input_array[_current_right];

			//printf("_tmp_arry:  ");
			//for (size_t i = 0; i <= last; i++)
			//	printf("%x  ", _tmp_array[i]);
			//printf("\n");

			//const auto endTime = high_resolution_clock::now();
			//print_results("Permutation: ", startTime, endTime);

			shiftRightAmount += BitsPerDigit;
			std::swap(_inout_array, _tmp_array);
			currentDigit++;
		}
	}
}

inline static void print_results(const char* const tag,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime) {
	printf("%s: Time: %fms\n", tag, duration_cast<duration<double, milli>>(endTime - startTime).count());
}

// Serial LSD Radix Sort. Baseline implementation - i.e. the slowest.
// Note: Only to be used with bitsPerDigit that are 1, 2, 4, 8 or 16, to ensure the result ends up in the input array, without needing to copy back from the output array.
template< unsigned BitsPerDigit >
inline void _RadixSortLSD_StableUnsigned_PowerOf2RadixScalar(unsigned* inout_array, unsigned* tmp_array, size_t last)
{
	static_assert((BitsPerDigit == 1 || BitsPerDigit == 2 || BitsPerDigit == 4 || BitsPerDigit == 8 || BitsPerDigit == 16), "BitsPerDigit must be 1, 2, 4, 8 or 16");

	const unsigned NumberOfBins = 1 << BitsPerDigit;
	const unsigned bitMask = NumberOfBins - 1;
	unsigned numberOfDigits = (sizeof(unsigned) * 8 + BitsPerDigit - 1) / BitsPerDigit;
	unsigned shiftRightAmount = 0;
	unsigned* _inout_array = inout_array;
	unsigned* _tmp_array   = tmp_array;
	size_t count[NumberOfBins], endOfBin[NumberOfBins];

	for (unsigned d = 0; d < numberOfDigits; d++)
	{
		for (unsigned i = 0; i < NumberOfBins; i++)     count[i] = 0;
		for (size_t _current = 0; _current <= last; _current++)	    // counting phase
			count[(_inout_array[_current] >> shiftRightAmount) & bitMask]++;

		endOfBin[0] = 0;
		for (unsigned i = 1; i < NumberOfBins; i++)
			endOfBin[i] = endOfBin[i - 1] + count[i - 1];

		for (size_t _current = 0; _current <= last; _current++)	// permutation phase
			_tmp_array[endOfBin[(_inout_array[_current] >> shiftRightAmount) & bitMask]++] = _inout_array[_current];

		shiftRightAmount += BitsPerDigit;
		std::swap(_inout_array, _tmp_array);
	}
}

// Serial LSD Radix Sort. Baseline implementation - i.e. the slowest.
// Note: Any number of bits per digit can be used (1-32).
// Result (sorted array) is returned in the inout_array.
template< unsigned BitsPerDigit >
inline void _RadixSortLSD_StableUnsigned_Nbit_PowerOf2RadixScalar(unsigned* inout_array, unsigned* tmp_array, size_t last)
{
	static_assert((BitsPerDigit > 1 && BitsPerDigit <= sizeof(unsigned) * 8), "BitsPerDigit must be 1-32");

	const unsigned NumberOfBins = 1 << BitsPerDigit;
	const unsigned bitMask = NumberOfBins - 1;
	unsigned numberOfDigits = (sizeof(unsigned) * 8 + BitsPerDigit - 1) / BitsPerDigit;
	unsigned shiftRightAmount = 0;
	unsigned* _inout_array = inout_array;
	unsigned* _tmp_array   = tmp_array;
	bool _tmp_array_has_result = false;
	size_t count[   NumberOfBins], endOfBin[NumberOfBins];

	for (unsigned d = 0; d < numberOfDigits; d++)
	{
		const auto startTime = high_resolution_clock::now();
		for (unsigned i = 0; i < NumberOfBins; i++)     count[i] = 0;
		for (size_t _current = 0; _current <= last; _current++)	    // counting phase
			count[(_inout_array[_current] >> shiftRightAmount) & bitMask]++;

		endOfBin[0] = 0;
		for (unsigned i = 1; i < NumberOfBins; i++)
			endOfBin[i] = endOfBin[i - 1] + count[i - 1];
		const auto endTime = high_resolution_clock::now();
		print_results("Histogram: ", startTime, endTime);

		const auto startTime1 = high_resolution_clock::now();
		for (size_t _current = 0; _current <= last; _current++)	// permutation phase
			_tmp_array[endOfBin[(_inout_array[_current] >> shiftRightAmount) & bitMask]++] = _inout_array[_current];
		const auto endTime1 = high_resolution_clock::now();
		print_results("Permute: ", startTime1, endTime1);

		shiftRightAmount += BitsPerDigit;
		std::swap(_inout_array, _tmp_array);
		_tmp_array_has_result = !_tmp_array_has_result;
	}
	if (_tmp_array_has_result)
		memcpy(_tmp_array, _inout_array, (last + 1) * sizeof(unsigned));
}
// LSD Radix Sort - stable sort.
// Baseline implementation - i.e. the slowest.
template< size_t Threshold = 100>
inline void RadixSortLSDPowerOf2Radix(unsigned* inout_array, unsigned* tmp_array, size_t inout_size)
{
	const unsigned long BitsPerDigit = 8;

	if (inout_size >= Threshold) {
		_RadixSortLSD_StableUnsigned_PowerOf2RadixScalar< BitsPerDigit >(inout_array, tmp_array, inout_size - 1);
		//ExperimentalSequentialAlgorithms::_RadixSortLSD_StableUnsigned_PowerOf2RadixScalar_1(inout_array, tmp_array, inout_size - 1);
		//ExperimentalSequentialAlgorithms::_RadixSortLSD_StableUnsigned_PowerOf2RadixScalar_4X(inout_array, tmp_array, inout_size - 1);
	}
	else {
		// TODO: Substitute Merge Sort, as it will get rid off the for loop, since it's internal to MergeSort
		insertionSortSimilarToSTLnoSelfAssignment(inout_array, inout_size);
		for (size_t j = 0; j < inout_size; j++)
			tmp_array[j] = inout_array[j];
	}
}
// LSD Radix Sort - stable sort.
// Baseline implementation - i.e. the slowest.
template< unsigned BitsPerDigit, size_t Threshold = 100>
inline void RadixSortLSDPowerOf2Radix_Nbit(unsigned* inout_array, unsigned* tmp_array, size_t inout_size)
{
	if (inout_size >= Threshold) {
		_RadixSortLSD_StableUnsigned_Nbit_PowerOf2RadixScalar< BitsPerDigit >(inout_array, tmp_array, inout_size - 1);
	}
	else {
		// TODO: Substitute Merge Sort, as it will get rid off the for loop, since it's internal to MergeSort
		insertionSortSimilarToSTLnoSelfAssignment(inout_array, inout_size);
		for (size_t j = 0; j < inout_size; j++)	// copy from input array to the destination array
			tmp_array[j] = inout_array[j];
	}
}

// Serial LSD Radix Sort, with Counting separated into its own phase, followed by a permutation phase, as is done in HPCsharp in C#
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, long Threshold>
inline void _RadixSortLSD_StableUnsigned_PowerOf2RadixScalar_TwoPhase(unsigned long long* input_array, unsigned long long* output_array, size_t last, unsigned long long bitMask, unsigned long shiftRightAmount, bool inputArrayIsDestination)
{
	const unsigned NumberOfBins = PowerOfTwoRadix;
	unsigned long long* _input_array = input_array;
	unsigned long long* _output_array = output_array;
	bool _output_array_has_result = false;
	unsigned currentDigit = 0;

	size_t** count2D = HistogramByteComponents <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(input_array, 0, last);

	while (bitMask != 0)						// end processing digits when all the mask bits have been processes and shift out, leaving none
	{
		size_t* count = count2D[currentDigit];

		size_t startOfBin[NumberOfBins];
		//long endOfBin[NumberOfBins];
		alignas(64) size_t endOfBin[NumberOfBins];
		//printf("endOfBin address = %p\n", endOfBin);
		startOfBin[0] = endOfBin[0] = 0;
		for (unsigned i = 1; i < NumberOfBins; i++)
			startOfBin[i] = endOfBin[i] = startOfBin[i - 1] + count[i - 1];

		for (size_t _current = 0; _current <= last; _current++)	// permutation phase
			_output_array[endOfBin[extractDigit(_input_array[_current], bitMask, shiftRightAmount)]++] = _input_array[_current];

		bitMask <<= Log2ofPowerOfTwoRadix;
		shiftRightAmount += Log2ofPowerOfTwoRadix;
		_output_array_has_result = !_output_array_has_result;
		std::swap(_input_array, _output_array);
		currentDigit++;
	}
	// Done with processing, copy all of the bins
	if (_output_array_has_result && inputArrayIsDestination)
		for (size_t _current = 0; _current <= last; _current++)	// copy from output array into the input array
			_input_array[_current] = _output_array[_current];
	if (!_output_array_has_result && !inputArrayIsDestination)
		for (size_t _current = 0; _current <= last; _current++)	// copy from input array back into the output array
			_output_array[_current] = _input_array[_current];

	const unsigned numberOfDigits = Log2ofPowerOfTwoRadix;	// deallocate 2D count array, which was allocated in Histogram
	for (unsigned i = 0; i < numberOfDigits; i++)
		delete[] count2D[i];
	delete[] count2D;
}

// Serial LSD Radix Sort, with Counting separated into its own phase, followed by a permutation phase, as is done in HPCsharp in C#
inline void _RadixSortLSD_StableUnsigned_PowerOf2RadixScalar_TwoPhase(unsigned* input_array, unsigned* output_array, size_t last, unsigned long shiftRightAmount)
{
	const unsigned BitsPerDigit = 8;
	const size_t NumberOfBins = 1 << BitsPerDigit;
	unsigned* _input_array = input_array;
	unsigned* _output_array = output_array;
	unsigned currentDigit = 0;
	unsigned maxDigit = sizeof(unsigned);
	const unsigned bit_mask = NumberOfBins - 1;

	//const auto startTime = high_resolution_clock::now();
	size_t* count2D = HistogramByteComponents(input_array, 0, last);
	//const auto endTime = high_resolution_clock::now();
	//print_results("Histogram: ", startTime, endTime);

	while (currentDigit < maxDigit)						// end processing digits when all the mask bits have been processes and shift out, leaving none
	{
		size_t* count = count2D + (currentDigit * NumberOfBins);  // TODO: Could move endOfBin calculation outside of this loop.
		alignas(64) size_t endOfBin[NumberOfBins];
		//printf("endOfBin address = %p\n", endOfBin);
		endOfBin[0] = 0;
		for (size_t i = 1; i < NumberOfBins; i++)
			endOfBin[i] = endOfBin[i - 1] + count[i - 1];

		//const auto startTime = high_resolution_clock::now();
		// permutation phase
#if 0
		for (size_t _current = 0; _current <= last; _current++)
			_output_array[endOfBin[(_input_array[_current] >> shiftRightAmount) & bit_mask]++] = _input_array[_current];
#else
		unsigned prev_digit = (_input_array[0] >> shiftRightAmount) & bit_mask;
		size_t index = endOfBin[prev_digit];
		_output_array[index++] = _input_array[0];
		for (size_t _current = 1; _current <= last; _current++)
		{
			unsigned digit = (_input_array[_current] >> shiftRightAmount) & bit_mask;
			if (digit != prev_digit)
			{
				endOfBin[prev_digit] = index;
				index = endOfBin[digit];
				prev_digit = digit;
			}
			_output_array[index++] = _input_array[_current];
		}
#endif
		//const auto endTime = high_resolution_clock::now();
		//print_results("Permutation: ", startTime, endTime);

		shiftRightAmount += BitsPerDigit;
		std::swap(_input_array, _output_array);
		currentDigit++;
	}
	delete[] count2D;
}
// Serial LSD Radix Sort, with Counting separated into its own phase, followed by a permutation phase, as is done in HPCsharp in C#
// Performance optimization of scanning the input array left-to-right followed by right-to-left in an alternating fashion to take advantage
// of evergrowing size of caches.
// Slower than the always left-to-right (incrementing addressing of the source) implementation - slightly.
inline void _RadixSortLSD_StableUnsigned_PowerOf2RadixScalar_TwoPhase_rlscan(unsigned* input_array, unsigned* output_array, size_t last, unsigned long shiftRightAmount)
{
	const unsigned BitsPerDigit = 8;
	const size_t NumberOfBins = 1 << BitsPerDigit;
	unsigned* _input_array = input_array;
	unsigned* _output_array = output_array;
	unsigned currentDigit = 0;
	unsigned maxDigit = sizeof(unsigned);
	const unsigned bit_mask = NumberOfBins - 1;

	//const auto startTime = high_resolution_clock::now();
	size_t* count2D = HistogramByteComponents(input_array, 0, last);
	//const auto endTime = high_resolution_clock::now();
	//print_results("Histogram: ", startTime, endTime);

	while (currentDigit < maxDigit)						// end processing digits when all the mask bits have been processes and shift out, leaving none
	{
		if ((currentDigit & 1) == 0)	// left-to-right for even digits
		{
			size_t* count = count2D + (currentDigit * NumberOfBins);  // TODO: Could move endOfBin calculation outside of this loop.
			alignas(64) size_t startOfBin[NumberOfBins];
			//printf("endOfBin address = %p\n", endOfBin);
			startOfBin[0] = 0;
			for (size_t i = 1; i < NumberOfBins; i++)
				startOfBin[i] = startOfBin[i - 1] + count[i - 1];

			//const auto startTime = high_resolution_clock::now();
			// permutation phase
			for (size_t _current = 0; _current <= last; _current++)
				_output_array[startOfBin[(_input_array[_current] >> shiftRightAmount) & bit_mask]++] = _input_array[_current];
			//const auto endTime = high_resolution_clock::now();
			//print_results("Permutation: ", startTime, endTime);
		}
		else     // right-to-left for odd digits
		{
			size_t* count = count2D + (currentDigit * NumberOfBins);  // TODO: Could move endOfBin calculation outside of this loop.
			alignas(64) size_t endOfBin[NumberOfBins];
			//printf("endOfBin address = %p\n", endOfBin);
			endOfBin[0] = count[0] - 1;
			for (size_t i = 1; i < NumberOfBins; i++)
				endOfBin[i] = endOfBin[i - 1] + count[i];

			//const auto startTime = high_resolution_clock::now();
			// permutation phase
			for (long long _current = last; _current >= 0; _current--)
				_output_array[endOfBin[(_input_array[_current] >> shiftRightAmount) & bit_mask]--] = _input_array[_current];
			//const auto endTime = high_resolution_clock::now();
			//print_results("Permutation: ", startTime, endTime);
		}
		shiftRightAmount += BitsPerDigit;
		std::swap(_input_array, _output_array);
		currentDigit++;
	}
	delete[] count2D;
}

// LSD Radix Sort - stable (LSD has to be, and this may preclude LSD Radix from being able to be in-place)
inline void RadixSortLSDPowerOf2Radix_unsigned_TwoPhase(unsigned* a, unsigned* b, size_t a_size)
{
	const unsigned long Threshold = 10;	// Threshold of when to switch to using Insertion Sort
	unsigned long shiftRightAmount = 0;

	// The beauty of using template arguments instead of function parameters for the Threshold and Log2ofPowerOfTwoRadix is
	// they are not pushed on the stack and are treated as constants, but local.
	if (a_size >= Threshold) {
		_RadixSortLSD_StableUnsigned_PowerOf2RadixScalar_TwoPhase(a, b, a_size - 1, shiftRightAmount);
		//_RadixSortLSD_StableUnsigned_PowerOf2RadixScalar_TwoPhase_rlscan(a, b, a_size - 1, shiftRightAmount);  // Slower than always left-to-right scan
	}
	else {
		// TODO: Substitute Merge Sort, as it will get rid off the for loop, since it's internal to MergeSort
		insertionSortSimilarToSTLnoSelfAssignment(a, a_size);
		for (size_t j = 0; j < a_size; j++)	// copy from input array to the destination array
			b[j] = a[j];
	}
}

// Permute phase of LSD Radix Sort with de-randomized write memory accesses
// Derandomizes system memory accesses by buffering all Radix bin accesses, turning 256-bin random memory writes into sequential writes
// Also implements an optimization for constant arrays, which avoids loop dependency of incrementing through memory/array access.
template< unsigned long BufferDepth >
inline void _RadixSortLSD_StableUnsigned_PowerOf2Radix_PermuteDerandomized_1(unsigned* input_array, unsigned* output_array,
	size_t startIndex, size_t endIndex, unsigned bitMask, unsigned shiftRightAmount,
	size_t* endOfBin, unsigned long numberOfBins, size_t bufferIndex[], unsigned bufferDerandomize[][BufferDepth])
{
	unsigned prev_digit = (input_array[startIndex] & bitMask) >> shiftRightAmount;
	size_t index = bufferIndex[prev_digit];
	bufferDerandomize[prev_digit][index++] = input_array[startIndex];
	for (size_t _current = startIndex + 1; _current <= endIndex; _current++)
	{
		unsigned digit = (input_array[_current] & bitMask ) >> shiftRightAmount;
		if (digit != prev_digit)
		{
			bufferIndex[prev_digit] = index;
			index = bufferIndex[digit];
			prev_digit = digit;
		}
		if (index < BufferDepth)
		{
			bufferDerandomize[digit][index++] = input_array[_current];
		}
		else
		{
			size_t outIndex = endOfBin[digit];
			unsigned* buff = &(bufferDerandomize[digit][0]);
#if 1
			memcpy(&(output_array[outIndex]), buff, BufferDepth * sizeof(unsigned));	// significantly faster than a for loop
			//nontemporal_memcpy_aligned(&(output_array[outIndex]), buff, BufferDepth * sizeof(unsigned));  // same speed as memcpy on laptop CPU
#else
			unsigned* outBuff = &(output_array[outIndex]);
			for (size_t i = 0; i < BufferDepth; i++)
				*outBuff++ = *buff++;
#endif
			endOfBin[digit] += BufferDepth;
			bufferDerandomize[digit][0] = input_array[_current];
			index = 1;
		}
	}
	bufferIndex[prev_digit] = index;
	// Flush all the derandomization buffers
	for (size_t whichBuff = 0; whichBuff < numberOfBins; whichBuff++)
	{
		size_t numOfElementsInBuff = bufferIndex[whichBuff];
		for (size_t i = 0; i < numOfElementsInBuff; i++)
			output_array[endOfBin[whichBuff]++] = bufferDerandomize[whichBuff][i];
		bufferIndex[whichBuff] = 0;
	}
}

// Permute phase of LSD Radix Sort with de-randomized write memory accesses
// Derandomizes system memory accesses by buffering all Radix bin accesses, turning 256-bin random memory writes into sequential writes
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, unsigned long BufferDepth>
inline void _RadixSortLSD_StableUnsigned_PowerOf2Radix_PermuteDerandomized(unsigned* input_array, unsigned* output_array, size_t startIndex, size_t endIndex, unsigned bitMask, unsigned shiftRightAmount,
	size_t endOfBin[], size_t bufferIndex[], unsigned bufferDerandomize[][BufferDepth])
{
	const unsigned long NumberOfBins = PowerOfTwoRadix;

	for (size_t _current = startIndex; _current <= endIndex; _current++)
	{
		unsigned digit = extractDigit(input_array[_current], bitMask, shiftRightAmount);
		if (bufferIndex[digit] < BufferDepth)
		{
			bufferDerandomize[digit][bufferIndex[digit]++] = input_array[_current];
		}
		else
		{
			size_t outIndex = endOfBin[digit];
			unsigned* buff  = &(bufferDerandomize[digit][0]);
#if 1
			memcpy(&(output_array[outIndex]), buff, BufferDepth * sizeof(unsigned));	// significantly faster than a for loop
#else
			unsigned* outBuff = &(output_array[outIndex]);
			for (size_t i = 0; i < BufferDepth; i++)
				*outBuff++ = *buff++;
#endif
			endOfBin[digit] += BufferDepth;
			bufferDerandomize[digit][0] = input_array[_current];
			bufferIndex[digit] = 1;
		}
	}
	// Flush all the derandomization buffers
	for (size_t whichBuff = 0; whichBuff < NumberOfBins; whichBuff++)
	{
		size_t numOfElementsInBuff = bufferIndex[whichBuff];
		for (size_t i = 0; i < numOfElementsInBuff; i++)
			output_array[endOfBin[whichBuff]++] = bufferDerandomize[whichBuff][i];
		bufferIndex[whichBuff] = 0;
	}
}

// Derandomizes system memory accesses by buffering all Radix bin accesses, turning 256-bin random memory writes into sequential writes
// Parallel LSD Radix Sort, with Counting separated into its own parallel phase, followed by a serial permutation phase, as is done in HPCsharp in C#
template< unsigned BitsPerDigit >
void _RadixSortLSD_StableUnsigned_PowerOf2Radix_TwoPhase_DeRandomize(unsigned* input_array, unsigned* output_array, size_t last, unsigned bitMask, unsigned long shiftRightAmount)
{
	const size_t NumberOfBins = 1 << BitsPerDigit;
	unsigned* _input_array  = input_array;
	unsigned* _output_array = output_array;
	bool _output_array_has_result = false;
	unsigned currentDigit = 0;
	static const size_t bufferDepth = 128;
#if 0
	__declspec(align(64)) unsigned bufferDerandomize[NumberOfBins][bufferDepth];
	__declspec(align(64)) size_t   bufferIndex[      NumberOfBins] = { 0 };
#else
	auto bufferDerandomize = new unsigned[NumberOfBins][bufferDepth];
	auto bufferIndex       = new size_t[  NumberOfBins] { 0 };
#endif
	//const auto startTime = high_resolution_clock::now();
	size_t* count2D = HistogramByteComponents(input_array, 0, last);
	//const auto endTime = high_resolution_clock::now();
	//print_results("Histogram: ", startTime, endTime);

	while (bitMask != 0)						// end processing digits when all the mask bits have been processes and shift out, leaving none
	{
		size_t* count = count2D + (currentDigit * NumberOfBins);

		size_t startOfBin[NumberOfBins], endOfBin[NumberOfBins];
		startOfBin[0] = endOfBin[0] = 0;
		for (size_t i = 1; i < NumberOfBins; i++)
			startOfBin[i] = endOfBin[i] = startOfBin[i - 1] + count[i - 1];

		//const auto startTime = high_resolution_clock::now();
		_RadixSortLSD_StableUnsigned_PowerOf2Radix_PermuteDerandomized_1< bufferDepth >(
			_input_array, _output_array, (size_t)0, last, bitMask, shiftRightAmount, endOfBin,
			NumberOfBins, bufferIndex, bufferDerandomize);
		//const auto endTime = high_resolution_clock::now();
		//print_results("Permutation: ", startTime, endTime);

		bitMask <<= BitsPerDigit;
		shiftRightAmount += BitsPerDigit;
		_output_array_has_result = !_output_array_has_result;
		std::swap(_input_array, _output_array);
		currentDigit++;
	}

	delete[] count2D;
#if 1
	delete[] bufferIndex;
	delete[] bufferDerandomize;
#endif
}

// Derandomizes system memory accesses by buffering all Radix bin accesses, turning 256-bin random memory writes into sequential writes
// Parallel LSD Radix Sort, with Counting separated into its own parallel phase, followed by a serial permutation phase, as is done in HPCsharp in C#
template< unsigned BitsPerDigit = 8 >
inline void _RadixSortLSD_StableUnsigned_Nbits_TwoPhase_DeRandomize(unsigned* input_array, unsigned* output_array, size_t last,
	unsigned bitMask, unsigned long shiftRightAmount, bool inputArrayIsDestination)
{
	const size_t NumberOfBins = (size_t)1 << BitsPerDigit;
	unsigned* _input_array  = input_array;
	unsigned* _output_array = output_array;
	bool _output_array_has_result = false;
	unsigned currentDigit = 0;
	static const size_t bufferDepth = 32;
#if 0
	__declspec(align(64)) unsigned bufferDerandomize[NumberOfBins][bufferDepth];
	__declspec(align(64)) size_t   bufferIndex[NumberOfBins] = { 0 };
#else
	auto bufferDerandomize = new unsigned[NumberOfBins][bufferDepth];
	auto bufferIndex       = new size_t[NumberOfBins]{ 0 };
#endif
	//const auto startTime = high_resolution_clock::now();
	size_t* count2D = HistogramNbitComponents(input_array, 0, last, BitsPerDigit);
	//const auto endTime = high_resolution_clock::now();
	//print_results("Histogram: ", startTime, endTime);

	size_t* startOfBin = new size_t[NumberOfBins];

	while (bitMask != 0)						// end processing digits when all the mask bits have been processes and shift out, leaving none
	{
		size_t* count = count2D + (currentDigit * NumberOfBins);

		startOfBin[0] = 0;
		for (size_t i = 1; i < NumberOfBins; i++)
			startOfBin[i] = startOfBin[i - 1] + count[i - 1];

		//const auto startTime = high_resolution_clock::now();
		_RadixSortLSD_StableUnsigned_PowerOf2Radix_PermuteDerandomized_1< bufferDepth >(
			_input_array, _output_array, (size_t)0, last, bitMask, shiftRightAmount, startOfBin,
			NumberOfBins, bufferIndex, bufferDerandomize);
		//const auto endTime = high_resolution_clock::now();
		//print_results("Permutation: ", startTime, endTime);

		bitMask <<= BitsPerDigit;
		shiftRightAmount += BitsPerDigit;
		_output_array_has_result = !_output_array_has_result;
		std::swap(_input_array, _output_array);
		currentDigit++;
	}
	// TODO: Optimize either in-place usage or not-inplace usage to eliminate copies for certain digit sizes
	// Done with processing, copy all of the bins
	if (_output_array_has_result && inputArrayIsDestination)  // TODO: replace with memcpy
	{
		//for (size_t _current = 0; _current <= last; _current++)	// copy from output array into the input array
		//	_output_array[_current] = _input_array[_current];
		memcpy(_output_array, _input_array, (last + 1) * sizeof(unsigned));	// significantly faster than a for loop
	}
	//if (!_output_array_has_result && !inputArrayIsDestination)
	//	for (size_t _current = 0; _current <= last; _current++)	// copy from input array back into the output array
	//		_input_array[_current] = _output_array[_current];

	delete[] startOfBin;
	delete[] count2D;
#if 1
	delete[] bufferIndex;
	delete[] bufferDerandomize;
#endif
}

// LSD Radix Sort - stable (LSD has to be, and this may preclude LSD Radix from being able to be in-place)
//template< unsigned Threshold = 100 >
inline void RadixSortLSDPowerOf2Radix_unsigned_TwoPhase_DeRandomize(unsigned* a, unsigned* b, size_t a_size)
{
	const unsigned Threshold = 100;	// Threshold of when to switch to using Insertion Sort
	const unsigned BitsPerDigit = 8;
	const unsigned PowerOfTwoRadix = 1 << BitsPerDigit;
	// Create bit-mask and shift right amount
	unsigned long shiftRightAmount = 0;
	unsigned bitMask = (unsigned)(((unsigned)(PowerOfTwoRadix - 1)) << shiftRightAmount);	// bitMask controls/selects how many and which bits we process at a time

	// The beauty of using template arguments instead of function parameters for the Threshold and Log2ofPowerOfTwoRadix is
	// they are not pushed on the stack and are treated as constants, but local.
	if (a_size >= Threshold) {
		_RadixSortLSD_StableUnsigned_PowerOf2Radix_TwoPhase_DeRandomize< BitsPerDigit >(a, b, a_size - 1, bitMask, shiftRightAmount);
	}
	else {
		// TODO: Substitute Merge Sort, as it will get rid off the for loop, since it's internal to MergeSort
		insertionSortSimilarToSTLnoSelfAssignment(a, a_size);
		for (unsigned long j = 0; j < a_size; j++)	// copy from input array to the destination array
			b[j] = a[j];
	}
}

// LSD Radix Sort - stable (LSD has to be, and this may preclude LSD Radix from being able to be in-place)
template< unsigned BitsPerDigit = 8, unsigned Threshold = 100 >
inline void RadixSortLSDPowerOf2Radix_Nbit_TwoPhase_DeRandomize(unsigned* a, unsigned* b, size_t a_size)
{
	const unsigned long PowerOfTwoRadix = 1UL << BitsPerDigit;
	// Create bit-mask and shift right amount
	unsigned long shiftRightAmount = 0;
	unsigned bitMask = (unsigned)(((unsigned)(PowerOfTwoRadix - 1)) << shiftRightAmount);	// bitMask controls/selects how many and which bits we process at a time

	// The beauty of using template arguments instead of function parameters for the Threshold and Log2ofPowerOfTwoRadix is
	// they are not pushed on the stack and are treated as constants, but local.
	if (a_size >= Threshold) {
		_RadixSortLSD_StableUnsigned_Nbits_TwoPhase_DeRandomize< BitsPerDigit >(
			a, b, a_size - 1, bitMask, shiftRightAmount, true);
	}
	else {
		// TODO: Substitute Merge Sort, as it will get rid off the for loop, since it's internal to MergeSort
		insertionSortSimilarToSTLnoSelfAssignment(a, a_size);
		for (unsigned long j = 0; j < a_size; j++)	// copy from input array to the destination array
			b[j] = a[j];
	}
}

// Stability is not needed when sorting an array of integers
// Post-allocation adaptivity, since the size of allocation is known in advance
inline void sort_radix_in_place_adaptive(unsigned* src, size_t src_size, double physical_memory_threshold_post = 0.75)
{
	size_t anticipated_memory_usage = sizeof(unsigned long) * src_size + physical_memory_used_in_megabytes();
	double physical_memory_fraction = (double)anticipated_memory_usage / (double)physical_memory_total_in_megabytes();
	printf("sort_radix_in_place_adaptive: physical memory used = %llu   physical memory total = %llu\n",
		physical_memory_used_in_megabytes(), physical_memory_total_in_megabytes());

	if (physical_memory_fraction > physical_memory_threshold_post)
	{
		printf("Running truly in-place MSD Radix Sort\n");
		hybrid_inplace_msd_radix_sort(src, src_size);		// in-place, not stable
	}
	else
	{
		unsigned* working_array = new(std::nothrow) unsigned[src_size];

		if (!working_array)
		{
			printf("Running truly in-place MSD Radix Sort\n");
			hybrid_inplace_msd_radix_sort(src, src_size);		// in-place, not stable
		}
		else
		{
			//for (size_t i = 0; i < src_size; i++)		// page in allocated array. Only then it shows up in memory usage measurements
			//	working_array[i] = (unsigned)i;

			//physical_memory_fraction = (double)physical_memory_used_in_megabytes() / (double)physical_memory_total_in_megabytes();
			//printf("sort_radix_in_place_adaptive #2: physical memory used = %llu   physical memory total = %llu\n",
			//	physical_memory_used_in_megabytes(), physical_memory_total_in_megabytes());

			printf("Running not-in-place LSD Radix Sort\n");
			RadixSortLSDPowerOf2Radix_unsigned_TwoPhase(src, working_array, src_size);	// not-in-place, stable
			delete[] working_array;
		}
	}
}

// l boundary is inclusive and r boundary is exclusive
template< class _Type >
inline void merge_sort_inplace_hybrid_with_insertion(_Type* src, size_t l, size_t r)
{
	if (r <= l) return;
	if ((r - l) <= 48) {
		insertionSortSimilarToSTLnoSelfAssignment(src + l, r - l);
		return;
	}
	size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;     // average without overflow

	merge_sort_inplace_hybrid_with_insertion(src, l, m);
	merge_sort_inplace_hybrid_with_insertion(src, m, r);

	//merge_in_place(src, l, m, r);       // merge the results (TODO: Needs size_t for arguments and modified to be truly in-place all the way down)
	std::inplace_merge(src + l, src + m, src + r);
}

inline void sort_radix_in_place_stable_adaptive(unsigned* src, size_t src_size, double physical_memory_threshold_post = 0.75)
{
	size_t memory_to_be_allocated_in_megabytes = src_size * sizeof(unsigned) / ((size_t)1024 * 1024);
	double physical_memory_fraction = (double)(physical_memory_used_in_megabytes() + memory_to_be_allocated_in_megabytes)
		                             / (double)physical_memory_total_in_megabytes();
	//printf("sort_radix_in_place_adaptive: physical memory used = %llu   physical memory total = %llu   to be allocated = %llu\n",
	//	physical_memory_used_in_megabytes(), physical_memory_total_in_megabytes(), memory_to_be_allocated_in_megabytes);

	if (physical_memory_fraction > physical_memory_threshold_post)
	{
		//printf("Running in-place stable adaptive sort\n");
		//std::stable_sort(src + 0, src + src_size);	// problematic as it is not purely in-place algorithm, which is what is needed to keep memory footprint low
		merge_sort_inplace_hybrid_with_insertion(src, 0, src_size);	// truly in-place
	}
	else
	{
		unsigned* working_array = new(std::nothrow) unsigned[src_size];

		if (!working_array)
		{
			//printf("Running truly in-place MSD Radix Sort\n");
			//std::stable_sort(src + 0, src + src_size);	// problematic as it is not purely in-place algorithm, which is what is needed to keep memory footprint low
			merge_sort_inplace_hybrid_with_insertion(src, 0, src_size);
		}
		else
		{
			//for (size_t i = 0; i < src_size; i++)		// page in allocated array. Only then it shows up in memory usage measurements
			//	working_array[i] = (unsigned long)i;

			//physical_memory_fraction = (double)physical_memory_used_in_megabytes() / (double)physical_memory_total_in_megabytes();
			//printf("sort_radix_in_place_adaptive #2: physical memory used = %llu   physical memory total = %llu\n",
			//	physical_memory_used_in_megabytes(), physical_memory_total_in_megabytes());

			//printf("Running not-in-place LSD Radix Sort\n");
			RadixSortLSDPowerOf2Radix_unsigned_TwoPhase(src, working_array, src_size);	// not-in-place, stable
			delete[] working_array;
		}
	}
}


#endif