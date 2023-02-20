// TODO: Try DeRandomized version, but without using Parallel Histogram
// TODO: For Parallel Hybrid Radix Sort, such as LSD Radix Sort, replace Insertion Sort with Parallel Merge Sort
#ifndef _RadixSortLsdParallel_h
#define _RadixSortLsdParallel_h

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

#include "RadixSortLSD.h"

using namespace tbb;

template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline unsigned long** HistogramByteComponentsParallel(unsigned long inArray[], int l, int r, int parallelThreshold = 64 * 1024)
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

	int m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;   // average without overflow

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

// Returns count[quanta][numberOfBins]
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline size_t** HistogramByteComponentsAcrossWorkQuantasQC(unsigned long inArray[], size_t l, size_t r, size_t workQuanta, size_t numberOfQuantas, int whichByte)
{
	const unsigned long numberOfBins = PowerOfTwoRadix;
	const unsigned long mask = 0xff;
	int shiftRightAmount = (int)(8 * whichByte);
	//cout << "HistogramQC: l = " << l << "  r = " << r << "  workQuanta = " << workQuanta << "  quanta = " << quanta << "  whichByte = " << whichByte << endl;

	size_t** count = new size_t* [numberOfQuantas];

	for (size_t i = 0; i < numberOfQuantas; i++)
	{
		count[i] = new size_t[numberOfBins];
		for (unsigned long j = 0; j < numberOfBins; j++)
			count[i][j] = 0;
	}

	if (l > r)
		return count;

	size_t startQuanta = l / workQuanta;
	size_t endQuanta = r / workQuanta;
	if (startQuanta == endQuanta)       // counting within a single workQuanta, either partial or full
	{
		size_t q = startQuanta;
		for (size_t currIndex = l; currIndex <= r; currIndex++)
		{
			unsigned int inByte = (inArray[currIndex] >> shiftRightAmount) & mask;
			count[q][inByte]++;
		}
	}
	else
	{
		size_t currIndex, endIndex;

		// process startQuanta, which is either partial or full
		size_t q = startQuanta;
		endIndex = startQuanta * workQuanta + (workQuanta - 1);
		for (currIndex = l; currIndex <= endIndex; currIndex++)
		{
			unsigned int inByte = (inArray[currIndex] >> shiftRightAmount) & mask;
			count[q][inByte]++;
		}

		// process endQuanta, which is either partial or full
		q = endQuanta;
		for (currIndex = endQuanta * workQuanta; currIndex <= r; currIndex++)
		{
			unsigned int inByte = (inArray[currIndex] >> shiftRightAmount) & mask;
			count[q][inByte]++;
		}

		// process full workQuantas > startQuanta and < endQuanta
		currIndex = (startQuanta + 1) * workQuanta;
		endQuanta--;
		for (q = startQuanta + 1; q <= endQuanta; q++)
		{
			for (size_t j = 0; j < workQuanta; j++)
			{
				unsigned int inByte = (inArray[currIndex++] >> shiftRightAmount) & mask;
				count[q][inByte]++;
			}
		}
	}

	return count;
}

template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline size_t** HistogramByteComponentsQCParInner(unsigned long inArray[], size_t l, size_t r, size_t workQuanta, size_t numberOfQuantas, int whichByte, size_t parallelThreshold = 16 * 1024)
{
	const unsigned long numberOfBins = PowerOfTwoRadix;
	size_t** countLeft = NULL;
	size_t** countRight = NULL;

	if (l > r)      // zero elements to compare
	{
		for (size_t i = 0; i < numberOfQuantas; i++)
		{
			countLeft[i] = new size_t[numberOfBins];
			for (unsigned long j = 0; j < numberOfBins; j++)
				countLeft[i][j] = 0;
		}
		return countLeft;
	}
	if ((r - l + 1) <= parallelThreshold)
		return HistogramByteComponentsAcrossWorkQuantasQC<PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l, r, workQuanta, numberOfQuantas, whichByte);

	size_t m = ((r + l) / 2);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	Concurrency::parallel_invoke(
#else
	tbb::parallel_invoke(
#endif
		[&] { countLeft  = HistogramByteComponentsQCParInner <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l,     m, workQuanta, numberOfQuantas, whichByte, parallelThreshold); },
		[&] { countRight = HistogramByteComponentsQCParInner <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, m + 1, r, workQuanta, numberOfQuantas, whichByte, parallelThreshold); }
	);
	// Combine left and right results (reduce step), only for workQuantas for which the counts were computed
	size_t startQuanta = l / workQuanta;
	size_t endQuanta = r / workQuanta;
	for (size_t i = startQuanta; i <= endQuanta; i++)
		for (unsigned long j = 0; j < numberOfBins; j++)
			countLeft[i][j] += countRight[i][j];

	for (size_t i = 0; i < numberOfQuantas; i++)
		delete[] countRight[i];
	delete[] countRight;

	return countLeft;
}

template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline size_t** HistogramByteComponentsQCPar(unsigned long* inArray, size_t l, size_t r, size_t workQuanta, size_t numberOfQuantas, unsigned long whichByte, size_t parallelThreshold = 16 * 1024)
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
#if 1
	return HistogramByteComponentsQCParInner<PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l, r, workQuanta, numberOfQuantas, whichByte, parallelThreshold);
#else
	return HistogramByteComponentsAcrossWorkQuantasQC<PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l, r, workQuanta, quanta, whichByte);
#endif
}

template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline size_t** ComputeStartOfBinsPar(unsigned long* inArray, size_t size, size_t workQuanta, size_t numberOfQuantas, unsigned long digit, size_t parallelThreshold = 16 * 1024)
{
	unsigned int numberOfBins = PowerOfTwoRadix;

	//unsigned long** count = HistogramByteComponentsAcrossWorkQuantasQC<PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, 0, size - 1, workQuanta, quanta, digit);
	size_t** count = HistogramByteComponentsQCPar<PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, 0, size - 1, workQuanta, numberOfQuantas, digit, parallelThreshold);

	parallelThreshold = 0;

	size_t** startOfBin = new size_t* [numberOfQuantas];     // start of bin for each parallel work item
	for (size_t q = 0; q < numberOfQuantas; q++)
	{
		startOfBin[q] = new size_t[numberOfBins];
		for (unsigned b = 0; b < numberOfBins; b++)
			startOfBin[q][b] = 0;
	}

	size_t* sizeOfBin = new size_t[numberOfBins];

	// Determine the overall size of each bin, across all work quantas
	for (unsigned int b = 0; b < numberOfBins; b++)
	{
		sizeOfBin[b] = 0;
		for (size_t q = 0; q < numberOfQuantas; q++)
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
	for (size_t q = 1; q < numberOfQuantas; q++)
		for (unsigned int b = 0; b < numberOfBins; b++)
		{
			startOfBin[q][b] = startOfBin[q - 1][b] + count[q - 1][b];
			//if (currDigit == 1)
			//    Console.WriteLine("ComputeStartOfBins: d = {0}  sizeOfBin[{1}][{2}] = {3}", currDigit, q, b, startOfBin[q][b]);
		}

	for (size_t q = 0; q < numberOfQuantas; q++)
		delete[] count[q];
	delete[] count;

	delete[] sizeOfBin;

	return startOfBin;
}

// Permute phase of LSD Radix Sort with de-randomized write memory accesses
// Derandomizes system memory accesses by buffering all Radix bin accesses, turning 256-bin random memory writes into sequential writes
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix>
inline void _RadixSortLSD_StableUnsigned_PowerOf2Radix_PermuteDerandomizedNew(
	unsigned long* inputArray, unsigned long* outputArray, size_t q, size_t** startOfBin, size_t startIndex, size_t endIndex,
	unsigned long bitMask, unsigned long shiftRightAmount, size_t** bufferIndex, unsigned long** bufferDerandomize, size_t* bufferIndexEnd, unsigned long BufferDepth)
{
	size_t* startOfBinLoc = startOfBin[q];
#if 1
	const unsigned long numberOfBins = PowerOfTwoRadix;

	size_t* bufferIndexLoc = bufferIndex[q];
	unsigned long* bufferDerandomizeLoc = bufferDerandomize[q];

	for (size_t currIndex = startIndex; currIndex < endIndex; currIndex++)
	{
		unsigned long currDigit = extractDigit(inputArray[currIndex], bitMask, shiftRightAmount);
		if (bufferIndexLoc[currDigit] < bufferIndexEnd[currDigit])
		{
			bufferDerandomizeLoc[bufferIndexLoc[currDigit]++] = inputArray[currIndex];
		}
		else
		{
			size_t outIndex = startOfBinLoc[currDigit];
			size_t buffIndex = currDigit * BufferDepth;
			memcpy(&(outputArray[outIndex]), &(bufferDerandomizeLoc[buffIndex]), BufferDepth * sizeof(unsigned long));	// significantly faster than a for loop
			startOfBinLoc[currDigit] += BufferDepth;
			bufferDerandomizeLoc[currDigit * BufferDepth] = inputArray[currIndex];
			bufferIndexLoc[currDigit] = currDigit * BufferDepth + 1;
		}
	}
	// Flush all the derandomization buffers
	for (unsigned long whichBuff = 0; whichBuff < numberOfBins; whichBuff++)
	{
		size_t outIndex       = startOfBinLoc[whichBuff];
		size_t buffStartIndex = whichBuff * BufferDepth;
		size_t buffEndIndex   = bufferIndexLoc[whichBuff];
		size_t numItems = (size_t)buffEndIndex - buffStartIndex;
		memcpy(&(outputArray[outIndex]), &(bufferDerandomizeLoc[buffStartIndex]), numItems * sizeof(unsigned long));
		bufferIndexLoc[whichBuff] = whichBuff * BufferDepth;
	}
#else
	// TODO: Figure out why this without-de-randomization version is not working correctly
	for (size_t _current = startIndex; _current <= endIndex; _current++)
		workArray[startOfBinLoc[extractDigit(inputArray[_current], bitMask, shiftRightAmount)]++] = inputArray[_current];
#endif
}

// This method is referenced in the Parallel LSD Radix Sort section of Practical Parallel Algorithms Book.
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline void SortRadixInnerPar(unsigned long* inputArray, unsigned long* workArray, size_t inputSize, size_t ParallelWorkQuantum = 64 * 1024)
{
	//unsigned int numberOfCores = std::thread::hardware_concurrency();
	const int NumberOfBins = PowerOfTwoRadix;
	bool outputArrayHasResult = false;
	size_t quanta = (inputSize % ParallelWorkQuantum) == 0 ? inputSize / ParallelWorkQuantum
		                                                   : inputSize / ParallelWorkQuantum + 1;
	// Setup de-randomization buffers for writes during the permutation phase
	const unsigned long BufferDepth = 64;
	unsigned long** bufferDerandomize = static_cast<unsigned long**>(operator new[](sizeof(unsigned long *) * quanta, (std::align_val_t)(64)));
	for (unsigned q = 0; q < quanta; q++)
		bufferDerandomize[q] = static_cast<unsigned long*>(operator new[](sizeof(unsigned long) * NumberOfBins * BufferDepth, (std::align_val_t)(64)));

	size_t** bufferIndex = static_cast<size_t**>(operator new[](sizeof(size_t*)* quanta, (std::align_val_t)(64)));
	for (size_t q = 0; q < quanta; q++)
	{
		bufferIndex[q] = static_cast<size_t*>(operator new[](sizeof(size_t) * NumberOfBins, (std::align_val_t)(64)));
		bufferIndex[q][0] = 0;
		for (int b = 1; b < NumberOfBins; b++)
			bufferIndex[q][b] = bufferIndex[q][b - 1] + BufferDepth;
	}
	//unsigned long* bufferIndexEnd = new(std::align_val_t{ 64 }) unsigned long[NumberOfBins];
	size_t* bufferIndexEnd = static_cast<size_t*>(operator new[](sizeof(size_t) * NumberOfBins, (std::align_val_t)(64)));
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
		size_t** startOfBin = ComputeStartOfBinsPar<PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inputArray, inputSize, ParallelWorkQuantum, quanta, digit);

		size_t numberOfFullQuantas = inputSize / ParallelWorkQuantum;
		size_t q;
		//cout << "NumberOfQuantas = " << quanta << "   NumberOfFullQuantas = " << numberOfFullQuantas << endl;
#if 0
		// Single core version of the algorithm
		for (q = 0; q < numberOfFullQuantas; q++)
		{
			size_t startIndex = q * ParallelWorkQuantum;
			size_t   endIndex = startIndex + ParallelWorkQuantum;	// non-inclusive

			_RadixSortLSD_StableUnsigned_PowerOf2Radix_PermuteDerandomizedNew<PowerOfTwoRadix, Log2ofPowerOfTwoRadix, BufferDepth>(
				inputArray, workArray, q, startOfBin, startIndex, endIndex, bitMask, shiftRightAmount, bufferIndex, bufferDerandomize, bufferIndexEnd);
		}
		if (quanta > numberOfFullQuantas)      // last partially filled workQuanta
		{
			size_t startIndex = q * ParallelWorkQuantum;
			size_t   endIndex = inputSize;									// non-inclusive
			_RadixSortLSD_StableUnsigned_PowerOf2Radix_PermuteDerandomizedNew<PowerOfTwoRadix, Log2ofPowerOfTwoRadix, BufferDepth>(
				inputArray, workArray, q, startOfBin, startIndex, endIndex, bitMask, shiftRightAmount, bufferIndex, bufferDerandomize, bufferIndexEnd);
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
			size_t startIndex = q * ParallelWorkQuantum;
			size_t   endIndex = startIndex + ParallelWorkQuantum;	// non-inclusive
			g.run([=] {																// important to not pass by reference, as all tasks will then get the same/last value
				_RadixSortLSD_StableUnsigned_PowerOf2Radix_PermuteDerandomizedNew<256, 8>(
					inputArray, workArray, q, startOfBin, startIndex, endIndex, bitMask, shiftRightAmount, bufferIndex, bufferDerandomize, bufferIndexEnd, BufferDepth);
				});
		}
		if (quanta > numberOfFullQuantas)      // last partially filled workQuantum
		{
			size_t startIndex = q * ParallelWorkQuantum;
			size_t   endIndex = inputSize;									// non-inclusive
			g.run([=] {
				_RadixSortLSD_StableUnsigned_PowerOf2Radix_PermuteDerandomizedNew<256, 8>(
					inputArray, workArray, q, startOfBin, startIndex, endIndex, bitMask, shiftRightAmount, bufferIndex, bufferDerandomize, bufferIndexEnd, BufferDepth);
				});
		}
		g.wait();
#endif
		bitMask <<= Log2ofPowerOfTwoRadix;
		digit++;
		shiftRightAmount += Log2ofPowerOfTwoRadix;
		outputArrayHasResult = !outputArrayHasResult;

		unsigned long* tmp = inputArray;       // swap input and output arrays
		inputArray = workArray;
		workArray = tmp;

		for (q = 0; q < quanta; q++)
			delete[] startOfBin[q];
		delete[] startOfBin;
	}
	//	if (outputArrayHasResult)
	//		for (unsigned long current = 0; current < inputSize; current++)		// copy from output array into the input array
	//			inputArray[current] = workArray[current];						// TODO: memcpy will most likely be faster
	//::operator delete[](bufferIndexEnd, std::align_val_t{ 64 });
	::operator delete[](bufferIndexEnd, std::align_val_t{ 64 });

	for (size_t q = 0; q < quanta; q++)
		::operator delete[](bufferIndex[q], std::align_val_t{ 64 });
	::operator delete[](bufferIndex, std::align_val_t{ 64 });

	for (size_t q = 0; q < quanta; q++)
		::operator delete[](bufferDerandomize[q], std::align_val_t{ 64 });
	::operator delete[](bufferDerandomize, std::align_val_t{ 64 });
}

// LSD Radix Sort - stable (LSD has to be, and this may preclude LSD Radix from being able to be in-place)
// Result is returned in "a", whereas "b" is used a temporary working buffer.
inline void SortRadixPar(unsigned long* a, size_t a_size, size_t parallelThreshold = 64 * 1024)
{
	const size_t Threshold = 100;	// Threshold of when to switch to using Insertion Sort
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
inline void SortRadixPar(unsigned long* a, unsigned long* tmp_work_buff, size_t a_size, size_t parallelThreshold = 512 * 1024)
{
	const size_t Threshold = 100;	// Threshold of when to switch to using Insertion Sort
	const unsigned long PowerOfTwoRadix = 256;
	const unsigned long Log2ofPowerOfTwoRadix = 8;

	// may return 0 when not able to detect
	auto processor_count = std::thread::hardware_concurrency();
	//printf("Number of cores = %u \n", processor_count);
	//processor_count = 16;

	if ((processor_count > 0) && (parallelThreshold * processor_count) < a_size)
		parallelThreshold = a_size / processor_count;

	// The beauty of using template arguments instead of function parameters for the Threshold and Log2ofPowerOfTwoRadix is
	// they are not pushed on the stack and are treated as constants, but local.
	if (a_size >= Threshold)
		SortRadixInnerPar< PowerOfTwoRadix, Log2ofPowerOfTwoRadix >(a, tmp_work_buff, a_size, parallelThreshold);
	else
		insertionSortSimilarToSTLnoSelfAssignment(a, a_size);	// TODO: Replace with Parallel Merge Sort to use a bigger Threshold, such at parallelThreshold
}

template< class _CountType >
class HistogramByteComponentsParallelType
{
	unsigned long* my_input_array;			// a local copy to the array being counted to provide a pointer to each parallel task
public:
	static const unsigned long numberOfDigits = 4;
	static const unsigned long numberOfBins = 256;
	alignas(64) _CountType my_count[numberOfDigits][numberOfBins];		// the count for this task

	HistogramByteComponentsParallelType(unsigned long* a) : my_input_array(a)	// constructor, which copies the pointer to the array being counted
	{
		for (unsigned long i = 0; i < numberOfDigits; i++)	// initialized all counts to zero, since the array may not contain all values
			for (unsigned long j = 0; j < numberOfBins; j++)
				my_count[i][j] = 0;
	}
	// Method that performs the core work of counting
	void operator()(const blocked_range< unsigned long >& r)
	{
		unsigned long* a = my_input_array;		// these local variables are used to help the compiler optimize the code better
		size_t         end = r.end();
		_CountType(*count)[numberOfBins] = my_count;
		_CountType* count0 = count[0];
		_CountType* count1 = count[1];
		_CountType* count2 = count[2];
		_CountType* count3 = count[3];
		for (size_t i = r.begin(); i != end; ++i)
		{
			unsigned long value = a[i];
			count0[value & 0xff]++;
			count1[(value >> 8) & 0xff]++;
			count2[(value >> 16) & 0xff]++;
			count3[(value >> 24) & 0xff]++;
		}
	}
	// Splitter (splitting constructor) required by the parallel_reduce
	// Takes a reference to the original object, and a dummy argument to distinguish this method from a copy constructor
	HistogramByteComponentsParallelType(HistogramByteComponentsParallelType& x, split) : my_input_array(x.my_input_array)
	{
		for (unsigned long i = 0; i < numberOfDigits; i++)	// initialized all counts to zero, since the array may not contain all values
			for (unsigned long j = 0; j < numberOfBins; j++)
				my_count[i][j] = 0;
	}
	// Join method required by parallel_reduce
	// Combines a result from another task into this result
	void join(const HistogramByteComponentsParallelType& y)
	{
		for (unsigned long i = 0; i < numberOfDigits; i++)
			for (unsigned long j = 0; j < numberOfBins; j++)
				my_count[i][j] += y.my_count[i][j];
	}
};

// Derandomizes system memory accesses by buffering all Radix bin accesses, turning 256-bin random memory writes into sequential writes
// Parallel LSD Radix Sort, with Counting separated into its own parallel phase, followed by a serial permutation phase, as is done in HPCsharp in C#
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, long Threshold>
void _RadixSortLSD_StableUnsigned_PowerOf2RadixParallel_TwoPhase_DeRandomize(unsigned long* input_array, unsigned long* output_array, long last, unsigned long bitMask, unsigned long shiftRightAmount, bool inputArrayIsDestination)
{
	const unsigned long numberOfBins = PowerOfTwoRadix;
	unsigned long* _input_array = input_array;
	unsigned long* _output_array = output_array;
	bool _output_array_has_result = false;
	unsigned long currentDigit = 0;
	static const unsigned long bufferDepth = 128;
	alignas(64) unsigned long bufferDerandomize[numberOfBins][bufferDepth];
	alignas(64) unsigned long bufferIndex[numberOfBins] = { 0 };

	//unsigned long** count2D = HistogramByteComponents <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inputArray, 0, endIndex);

	HistogramByteComponentsParallelType<unsigned long> histogramParallel(input_array);	// contains the count array, which is initialized to all zeros

	parallel_reduce(blocked_range< unsigned long >(0, last + 1), histogramParallel);

	while (bitMask != 0)						// end processing digits when all the mask bits have been processes and shift out, leaving none
	{
		unsigned long* count = histogramParallel.my_count[currentDigit];

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
inline void RadixSortLSDPowerOf2RadixParallel_unsigned_TwoPhase_DeRandomize(unsigned long* a, unsigned long* b, unsigned long a_size)
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
		_RadixSortLSD_StableUnsigned_PowerOf2RadixParallel_TwoPhase_DeRandomize< PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(a, b, a_size - 1, bitMask, shiftRightAmount, false);
	}
	else {
		// TODO: Substitute Merge Sort, as it will get rid off the for loop, since it's internal to MergeSort
		insertionSortSimilarToSTLnoSelfAssignment(a, a_size);
		for (unsigned long j = 0; j < a_size; j++)	// copy from input array to the destination array
			b[j] = a[j];
	}
}

#endif