#pragma once

#include "InsertionSort.h"
#include "BinarySearch.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
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

template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline unsigned long** HistogramByteComponentsParallel(unsigned long inArray[], int l, int r, int parallelThreshold = 16 * 1024)
{
	const unsigned long numberOfDigits = Log2ofPowerOfTwoRadix;
	const unsigned long numberOfBins   = PowerOfTwoRadix;

	unsigned long** countLeft;
	unsigned long** countRight;

	if (l > r)      // zero elements to compare
	{
		countLeft = new unsigned long* [numberOfDigits];

		for (unsigned long i = 0; i < numberOfDigits; i++)
		{
			countLeft[i] = new unsigned long[numberOfBins];
			for (unsigned long j = 0; j < numberOfBins; j++)
				countLeft[i][j] = 0;
		}
		return countLeft;
	}
	if ((r - l + 1) <= parallelThreshold)
	{
		countLeft = new unsigned long* [numberOfDigits];

		for (unsigned long i = 0; i < numberOfDigits; i++)
		{
			countLeft[i] = new unsigned long[numberOfBins];
			for (unsigned long j = 0; j < numberOfBins; j++)
				countLeft[i][j] = 0;
		}
		// Faster version, since it doesn't use a 2-D array, reducing one level of indirection
		unsigned long* count0 = countLeft[0];
		unsigned long* count1 = countLeft[1];
		unsigned long* count2 = countLeft[2];
		unsigned long* count3 = countLeft[3];
#if 1
		for (int current = l; current <= r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
		{
			unsigned long value = inArray[current];
			count0[value         & 0xff]++;
			count1[(value >>  8) & 0xff]++;
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

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	Concurrency::parallel_invoke(
#else
	tbb::parallel_invoke(
#endif
		[&] { countLeft  = HistogramByteComponentsParallel <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l,     m, parallelThreshold); },
		[&] { countRight = HistogramByteComponentsParallel <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, m + 1, r, parallelThreshold); }
	);
	// Combine left and right results
	for (unsigned long i = 0; i < numberOfDigits; i++)
		for (unsigned long j = 0; j < numberOfBins; j++)
			countLeft[i][j] += countRight[i][j];

	for (unsigned long i = 0; i < numberOfDigits; i++)
		delete[] countRight[i];
	delete[] countRight;

	return countLeft;
}

// Serial LSD Radix Sort, with Counting separated into its own phase, followed by a permutation phase, as is done in HPCsharp in C#
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, long Threshold>
inline void _RadixSortLSD_StableUnsigned_PowerOf2RadixParallel_TwoPhase(unsigned long* input_array, unsigned long* output_array, long last, unsigned long bitMask, unsigned long shiftRightAmount, bool inputArrayIsDestination)
{
	const unsigned long numberOfBins   = PowerOfTwoRadix;
	const unsigned long numberOfDigits = Log2ofPowerOfTwoRadix;
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
	for (unsigned long i = 0; i < numberOfDigits; i++)
		delete[] count2D[i];
	delete[] count2D;
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

// Returns count[numberOfQuantas][numberOfBins]
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline unsigned long** HistogramByteComponentsAcrossWorkQuantasQC(unsigned long inArray[], int l, int r, int workQuanta, int numberOfQuantas, int whichByte)
{
	const unsigned long numberOfBins = PowerOfTwoRadix;
	const unsigned long mask = 0xff;
	int shiftRightAmount = (int)(8 * whichByte);
	//cout << "HistogramQC: l = " << l << "  r = " << r << "  workQuanta = " << workQuanta << "  numberOfQuantas = " << numberOfQuantas << "  whichByte = " << whichByte << endl;

	unsigned long** count = new unsigned long* [numberOfQuantas];

	for (int i = 0; i < numberOfQuantas; i++)
	{
		count[i] = new unsigned long[numberOfBins];
		for (unsigned long j = 0; j < numberOfBins; j++)
			count[i][j] = 0;
	}

	if (l > r)
		return count;

	long startQuanta = l / workQuanta;
	long endQuanta = r / workQuanta;
	if (startQuanta == endQuanta)       // counting within a single workQuanta, either partial or full
	{
		long q = startQuanta;
		for (int currIndex = l; currIndex <= r; currIndex++)
		{
			unsigned int inByte = (inArray[currIndex] >> shiftRightAmount) & mask;
			count[q][inByte]++;
		}
	}
	else
	{
		int currIndex, endIndex;

		// process startQuanta, which is either partial or full
		long q = startQuanta;
		endIndex = (long)(startQuanta * workQuanta + (workQuanta - 1));
		for (currIndex = l; currIndex <= endIndex; currIndex++)
		{
			unsigned int inByte = (inArray[currIndex] >> shiftRightAmount) & mask;
			count[q][inByte]++;
		}

		// process endQuanta, which is either partial or full
		q = endQuanta;
		for (currIndex = (int)(endQuanta * workQuanta); currIndex <= r; currIndex++)
		{
			unsigned int inByte = (inArray[currIndex] >> shiftRightAmount) & mask;
			count[q][inByte]++;
		}

		// process full workQuantas > startQuanta and < endQuanta
		currIndex = (int)((startQuanta + 1) * workQuanta);
		endQuanta--;
		for (q = (int)(startQuanta + 1); q <= endQuanta; q++)
		{
			for (int j = 0; j < workQuanta; j++)
			{
				unsigned int inByte = (inArray[currIndex++] >> shiftRightAmount) & mask;
				count[q][inByte]++;
			}
		}
	}

	return count;
}

template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline unsigned long** HistogramByteComponentsQCParInner(unsigned long inArray[], int l, int r, int workQuanta, int numberOfQuantas, int whichByte, int parallelThreshold = 16 * 1024)
{
	const unsigned long numberOfBins = PowerOfTwoRadix;
	unsigned long** countLeft = NULL;
	unsigned long** countRight = NULL;

	if (l > r)      // zero elements to compare
	{
		for (int i = 0; i < numberOfQuantas; i++)
		{
			countLeft[i] = new unsigned long[numberOfBins];
			for (unsigned long j = 0; j < numberOfBins; j++)
				countLeft[i][j] = 0;
		}
		return countLeft;
	}
	if ((r - l + 1) <= parallelThreshold)
		return HistogramByteComponentsAcrossWorkQuantasQC<PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l, r, workQuanta, numberOfQuantas, whichByte);

	int m = ((r + l) / 2);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	Concurrency::parallel_invoke(
#else
	tbb::parallel_invoke(
#endif
		[&] { countLeft  = HistogramByteComponentsQCParInner <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l,     m, workQuanta, numberOfQuantas, whichByte, parallelThreshold); },
		[&] { countRight = HistogramByteComponentsQCParInner <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, m + 1, r, workQuanta, numberOfQuantas, whichByte, parallelThreshold); }
	);
	// Combine left and right results (reduce step), only for workQuantas for which the counts were computed
	long startQuanta = l / workQuanta;
	long endQuanta = r / workQuanta;
	for (long i = startQuanta; i <= endQuanta; i++)
		for (unsigned long j = 0; j < numberOfBins; j++)
			countLeft[i][j] += countRight[i][j];

	for (int i = 0; i < numberOfQuantas; i++)
		delete[] countRight[i];
	delete[] countRight;

	return countLeft;
}

template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline unsigned long** HistogramByteComponentsQCPar(unsigned long* inArray, long l, long r, int workQuanta, int numberOfQuantas, unsigned long whichByte, int parallelThreshold = 16 * 1024)
{
	//may return 0 when not able to detect
	auto processor_count = std::thread::hardware_concurrency();
	if (processor_count < 1)
	{
		processor_count = 1;
		//cout << "Warning: Fewer than 1 processor core detected. Using only a single core.";
	}

	long length = r - l + 1;

	if ((long)(parallelThreshold * processor_count) < length)
		parallelThreshold = length / processor_count;
#if 1
	return HistogramByteComponentsQCParInner<PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l, r, workQuanta, numberOfQuantas, whichByte, parallelThreshold);
#else
	return HistogramByteComponentsAcrossWorkQuantasQC<PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l, r, workQuanta, numberOfQuantas, whichByte);
#endif
}

template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline unsigned long** ComputeStartOfBinsPar(unsigned long* inArray, unsigned long size, unsigned long workQuanta, unsigned long numberOfQuantas, unsigned long digit, int parallelThreshold = 16 * 1024)
{
	unsigned int numberOfBins = PowerOfTwoRadix;

	//unsigned long** count = HistogramByteComponentsAcrossWorkQuantasQC<PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, 0, size - 1, workQuanta, numberOfQuantas, digit);
	unsigned long** count = HistogramByteComponentsQCPar<PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, 0, size - 1, workQuanta, numberOfQuantas, digit, parallelThreshold);

	parallelThreshold = 0;

	unsigned long** startOfBin = new unsigned long* [numberOfQuantas];     // start of bin for each parallel work item
	for (unsigned q = 0; q < numberOfQuantas; q++)
	{
		startOfBin[q] = new unsigned long[numberOfBins];
		for (unsigned b = 0; b < numberOfBins; b++)
			startOfBin[q][b] = 0;
	}

	unsigned int* sizeOfBin = new unsigned int[numberOfBins];

	// Determine the overall size of each bin, across all work quantas
	for (unsigned int b = 0; b < numberOfBins; b++)
	{
		sizeOfBin[b] = 0;
		for (unsigned q = 0; q < numberOfQuantas; q++)
		{
			sizeOfBin[b] += count[q][b];
			//cout << "count[" << q << "][" << b << "] = " << count[q][b] << "  ";
		}
		//cout << endl;
		//cout << "ComputeStartOfBins: d = " << digit << "  sizeOfBin[" << b << "] = " << sizeOfBin[b] << endl;
	}

	// Determine starting of bins for work quanta 0
	startOfBin[0][0] = 0;
	for (unsigned int b = 1; b < numberOfBins; b++)
	{
		startOfBin[0][b] = startOfBin[0][b - 1] + sizeOfBin[b - 1];
		//cout << "ComputeStartOfBins: d = " << digit << "  startOfBin[0][" << b << "] = " << startOfBin[0][b] << endl;
	}

	// Determine starting of bins for work quanta 1 thru Q
	for (unsigned int q = 1; q < numberOfQuantas; q++)
		for (unsigned int b = 0; b < numberOfBins; b++)
		{
			startOfBin[q][b] = startOfBin[q - 1][b] + count[q - 1][b];
			//if (currDigit == 1)
			//    Console.WriteLine("ComputeStartOfBins: d = {0}  sizeOfBin[{1}][{2}] = {3}", currDigit, q, b, startOfBin[q][b]);
		}

	for (unsigned q = 0; q < numberOfQuantas; q++)
		delete[] count[q];
	delete[] count;

	delete[] sizeOfBin;

	return startOfBin;
}

// Permute phase of LSD Radix Sort with de-randomized write memory accesses
// Derandomizes system memory accesses by buffering all Radix bin accesses, turning 256-bin random memory writes into sequential writes
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix>
inline void _RadixSortLSD_StableUnsigned_PowerOf2Radix_PermuteDerandomizedNew(
	unsigned long* inputArray, unsigned long* outputArray, unsigned long q, unsigned long** startOfBin, unsigned long startIndex, unsigned long endIndex,
	unsigned long bitMask, unsigned long shiftRightAmount, unsigned long** bufferIndex, unsigned long** bufferDerandomize, unsigned long* bufferIndexEnd, unsigned long BufferDepth)
{
	const unsigned long numberOfBins = PowerOfTwoRadix;

	unsigned long* startOfBinLoc = startOfBin[q];
	unsigned long* bufferIndexLoc = bufferIndex[q];
	unsigned long* bufferDerandomizeLoc = bufferDerandomize[q];

	for (unsigned long currIndex = startIndex; currIndex < endIndex; currIndex++)
	{
		unsigned long currDigit = extractDigit(inputArray[currIndex], bitMask, shiftRightAmount);
		if (bufferIndexLoc[currDigit] < bufferIndexEnd[currDigit])
		{
			bufferDerandomizeLoc[bufferIndexLoc[currDigit]++] = inputArray[currIndex];
		}
		else
		{
			unsigned long outIndex = startOfBinLoc[currDigit];
			unsigned long buffIndex = currDigit * BufferDepth;
			memcpy(&(outputArray[outIndex]), &(bufferDerandomizeLoc[buffIndex]), BufferDepth * sizeof(unsigned long));	// significantly faster than a for loop
			startOfBinLoc[currDigit] += BufferDepth;
			bufferDerandomizeLoc[currDigit * BufferDepth] = inputArray[currIndex];
			bufferIndexLoc[currDigit] = currDigit * BufferDepth + 1;
		}
	}
	// Flush all the derandomization buffers
	for (unsigned long whichBuff = 0; whichBuff < numberOfBins; whichBuff++)
	{
		unsigned long outIndex       = startOfBinLoc[whichBuff];
		unsigned long buffStartIndex = whichBuff * BufferDepth;
		unsigned long buffEndIndex   = bufferIndexLoc[whichBuff];
		size_t numItems = (size_t)buffEndIndex - buffStartIndex;
		memcpy(&(outputArray[outIndex]), &(bufferDerandomizeLoc[buffStartIndex]), numItems * sizeof(unsigned long));
		bufferIndexLoc[whichBuff] = whichBuff * BufferDepth;
	}
}


template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline void SortRadixInnerPar(unsigned long* inputArray, unsigned long* outputArray, long inputSize, long SortRadixParallelWorkQuanta = 64 * 1024)
{
	//unsigned int numberOfCores = std::thread::hardware_concurrency();
	const int NumberOfBins = PowerOfTwoRadix;
	bool outputArrayHasResult = false;
	unsigned int numberOfQuantas = (inputSize % SortRadixParallelWorkQuanta) == 0 ? (unsigned int)(inputSize / SortRadixParallelWorkQuanta)
		: (unsigned int)(inputSize / SortRadixParallelWorkQuanta + 1);
	// Setup de-randomization buffers for writes during the permutation phase
	const unsigned long BufferDepth = 64;
	unsigned long** bufferDerandomize = static_cast<unsigned long**>(operator new[](sizeof(unsigned long *) * numberOfQuantas, (std::align_val_t)(64)));
	for (unsigned q = 0; q < numberOfQuantas; q++)
		bufferDerandomize[q] = static_cast<unsigned long*>(operator new[](sizeof(unsigned long) * NumberOfBins * BufferDepth, (std::align_val_t)(64)));

	unsigned long** bufferIndex = static_cast<unsigned long**>(operator new[](sizeof(unsigned long*)* numberOfQuantas, (std::align_val_t)(64)));
	for (unsigned q = 0; q < numberOfQuantas; q++)
	{
		bufferIndex[q] = static_cast<unsigned long*>(operator new[](sizeof(unsigned long) * NumberOfBins, (std::align_val_t)(64)));
		bufferIndex[q][0] = 0;
		for (int b = 1; b < NumberOfBins; b++)
			bufferIndex[q][b] = bufferIndex[q][b - 1] + BufferDepth;
	}
	//unsigned long* bufferIndexEnd = new(std::align_val_t{ 64 }) unsigned long[NumberOfBins];
	unsigned long* bufferIndexEnd = static_cast<unsigned long*>(operator new[](sizeof(unsigned long) * NumberOfBins, (std::align_val_t)(64)));
	bufferIndexEnd[0] = BufferDepth;									// non-inclusive
	for (int b = 1; b < NumberOfBins; b++)
		bufferIndexEnd[b] = bufferIndexEnd[b - 1] + BufferDepth;
	// End of de-randomization buffers setup

	if (inputSize == 0)
		return;

	// Use TPL ideas from https://docs.microsoft.com/en-us/dotnet/standard/parallel-programming/task-based-asynchronous-programming

	unsigned long bitMask = 255;
	int shiftRightAmount = 0;
	unsigned int digit = 0;

	while (bitMask != 0)    // end processing digits when all the mask bits have been processed and shifted out, leaving no bits set in the bitMask
	{
		unsigned long** startOfBin = ComputeStartOfBinsPar<PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inputArray, inputSize, SortRadixParallelWorkQuanta, numberOfQuantas, digit);

		unsigned long numberOfFullQuantas = inputSize / SortRadixParallelWorkQuanta;
		unsigned long q;
		//cout << "NumberOfQuantas = " << numberOfQuantas << "   NumberOfFullQuantas = " << numberOfFullQuantas << endl;
#if 0
		// Single core version of the algorithm
		for (q = 0; q < numberOfFullQuantas; q++)
		{
			unsigned long startIndex = q * SortRadixParallelWorkQuanta;
			unsigned long   endIndex = startIndex + SortRadixParallelWorkQuanta;	// non-inclusive

			_RadixSortLSD_StableUnsigned_PowerOf2Radix_PermuteDerandomizedNew<PowerOfTwoRadix, Log2ofPowerOfTwoRadix, BufferDepth>(
				inputArray, outputArray, q, startOfBin, startIndex, endIndex, bitMask, shiftRightAmount, bufferIndex, bufferDerandomize, bufferIndexEnd);
		}
		if (numberOfQuantas > numberOfFullQuantas)      // last partially filled workQuanta
		{
			unsigned long startIndex = q * SortRadixParallelWorkQuanta;
			unsigned long   endIndex = inputSize;									// non-inclusive
			_RadixSortLSD_StableUnsigned_PowerOf2Radix_PermuteDerandomizedNew<PowerOfTwoRadix, Log2ofPowerOfTwoRadix, BufferDepth>(
				inputArray, outputArray, q, startOfBin, startIndex, endIndex, bitMask, shiftRightAmount, bufferIndex, bufferDerandomize, bufferIndexEnd);
		}
#else
		// Multi-core version of the algorithm
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		Concurrency::task_group g;
#else
		tbb::task_group g;
#endif
		for (q = 0; q < numberOfFullQuantas; q++)
		{
			unsigned long startIndex = q * SortRadixParallelWorkQuanta;
			unsigned long   endIndex = startIndex + SortRadixParallelWorkQuanta;	// non-inclusive
			g.run([=] {																// important to not pass by reference, as all tasks will then get the same/last value
				_RadixSortLSD_StableUnsigned_PowerOf2Radix_PermuteDerandomizedNew<256, 8>(
					inputArray, outputArray, q, startOfBin, startIndex, endIndex, bitMask, shiftRightAmount, bufferIndex, bufferDerandomize, bufferIndexEnd, BufferDepth);
				});
		}
		if (numberOfQuantas > numberOfFullQuantas)      // last partially filled workQuanta
		{
			unsigned long startIndex = q * SortRadixParallelWorkQuanta;
			unsigned long   endIndex = inputSize;									// non-inclusive
			g.run([=] {
				_RadixSortLSD_StableUnsigned_PowerOf2Radix_PermuteDerandomizedNew<256, 8>(
					inputArray, outputArray, q, startOfBin, startIndex, endIndex, bitMask, shiftRightAmount, bufferIndex, bufferDerandomize, bufferIndexEnd, BufferDepth);
				});
		}
		g.wait();
#endif
		bitMask <<= Log2ofPowerOfTwoRadix;
		digit++;
		shiftRightAmount += Log2ofPowerOfTwoRadix;
		outputArrayHasResult = !outputArrayHasResult;

		unsigned long* tmp = inputArray;       // swap input and output arrays
		inputArray = outputArray;
		outputArray = tmp;

		for (q = 0; q < numberOfQuantas; q++)
			delete[] startOfBin[q];
		delete[] startOfBin;
	}
	//	if (outputArrayHasResult)
	//		for (unsigned long current = 0; current < inputSize; current++)		// copy from output array into the input array
	//			inputArray[current] = outputArray[current];						// TODO: memcpy will most likely be faster
	//::operator delete[](bufferIndexEnd, std::align_val_t{ 64 });
	::operator delete[](bufferIndexEnd, std::align_val_t{ 64 });

	for (unsigned q = 0; q < numberOfQuantas; q++)
		::operator delete[](bufferIndex[q], std::align_val_t{ 64 });
	::operator delete[](bufferIndex, std::align_val_t{ 64 });

	for (unsigned q = 0; q < numberOfQuantas; q++)
		::operator delete[](bufferDerandomize[q], std::align_val_t{ 64 });
	::operator delete[](bufferDerandomize, std::align_val_t{ 64 });
}

// LSD Radix Sort - stable (LSD has to be, and this may preclude LSD Radix from being able to be in-place)
// Result is returned in "a", whereas "b" is used a temporary working buffer.
inline void SortRadixPar(unsigned long* a, unsigned long a_size, unsigned parallelThreshold = 64 * 1024)
{
	const unsigned long Threshold = 100;	// Threshold of when to switch to using Insertion Sort
	const unsigned long PowerOfTwoRadix = 256;
	const unsigned long Log2ofPowerOfTwoRadix = 8;

	unsigned long* b = new unsigned long[a_size];		// this allocation does slow things down a bit. If we want even faster, then pass "b" in as an argument

	// may return 0 when not able to detect
	auto processor_count = std::thread::hardware_concurrency();
	//printf("Number of cores = %u \n", processor_count);
	processor_count *= 4;									// Increase the number of cores to split array into more pieces than cores, which increases performance

	if ((processor_count > 0) && (parallelThreshold * processor_count) < a_size)
		parallelThreshold = a_size / processor_count;

	// The beauty of using template arguments instead of function parameters for the Threshold and Log2ofPowerOfTwoRadix is
	// they are not pushed on the stack and are treated as constants, but local.
	if (a_size >= Threshold)
		SortRadixInnerPar< PowerOfTwoRadix, Log2ofPowerOfTwoRadix >(a, b, a_size, parallelThreshold);
	else
		insertionSortSimilarToSTLnoSelfAssignment(a, a_size);

	delete[] b;
}

// Faster implementation, when the user is willing to provide a pre-alocated temporary/working buffer, which makes it a bit more cumbersome to use
inline void SortRadixPar(unsigned long* a, unsigned long* tmp_work_buff, unsigned long a_size, unsigned parallelThreshold = 64 * 1024)
{
	const unsigned long Threshold = 100;	// Threshold of when to switch to using Insertion Sort
	const unsigned long PowerOfTwoRadix = 256;
	const unsigned long Log2ofPowerOfTwoRadix = 8;

	// may return 0 when not able to detect
	auto processor_count = std::thread::hardware_concurrency();
	//printf("Number of cores = %u \n", processor_count);
	//processor_count = 8;									// Fastest on 48-core AWS Intel machine

	if ((processor_count > 0) && (parallelThreshold * processor_count) < a_size)
		parallelThreshold = a_size / processor_count;

	// The beauty of using template arguments instead of function parameters for the Threshold and Log2ofPowerOfTwoRadix is
	// they are not pushed on the stack and are treated as constants, but local.
	if (a_size >= Threshold)
		SortRadixInnerPar< PowerOfTwoRadix, Log2ofPowerOfTwoRadix >(a, tmp_work_buff, a_size, parallelThreshold);
	else
		insertionSortSimilarToSTLnoSelfAssignment(a, a_size);
}

