#pragma once

#include "Configuration.h"

#include <iostream>
#include <algorithm>
#include <random>
#include <ratio>
#include <vector>
#include <thread>
#include <execution>

namespace ParallelAlgorithms
{
	// left (l) boundary is inclusive and right (r) boundary is exclusive
	template< unsigned NumberOfBins >
	inline size_t* HistogramOneByteComponentParallel(unsigned char inArray[], size_t l, size_t r, size_t parallelThreshold = 64 * 1024)
	{
		size_t* countLeft  = NULL;
		size_t* countRight = NULL;

		if (l >= r)      // zero elements to compare
		{
			countLeft = new size_t[NumberOfBins]{};
			return countLeft;
		}
		if ((r - l) <= parallelThreshold)
		{
			countLeft = new size_t[NumberOfBins]{};

			for (size_t current = l; current < r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
				countLeft[inArray[current]]++;

			return countLeft;
		}

		size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;  // average without overflow

#if defined(USE_PPL)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { countLeft  = HistogramOneByteComponentParallel <NumberOfBins>(inArray, l, m, parallelThreshold); },
			[&] { countRight = HistogramOneByteComponentParallel <NumberOfBins>(inArray, m, r, parallelThreshold); }
		);
		// Combine left and right results
		for (size_t j = 0; j < NumberOfBins; j++)
			countLeft[j] += countRight[j];

		delete[] countRight;

		return countLeft;
	}

	// left (l) boundary is inclusive and right (r) boundary is exclusive
	template< unsigned NumberOfBins >
	inline size_t* HistogramOneByteComponentParallel_4(unsigned char inArray[], size_t l, size_t r, size_t parallelThreshold = 64 * 1024)
	{
		size_t* countLeft  = NULL;
		size_t* countRight = NULL;

		if (l >= r)      // zero elements to compare
		{
			countLeft = new size_t[NumberOfBins]{};
			return countLeft;
		}
		if ((r - l) <= parallelThreshold)
		{
			countLeft = new size_t[NumberOfBins]{};
			size_t current = l;
			//if (((unsigned long long)(inArray + l) & 0x7) != 0)
			//	printf("Memory alignment is not on 8-byte boundary\n");
			// TODO: Detect not-64-bit aligned address and process bytes individually until alignment is achieved and then do 64-bits at a time
			size_t last_by_eight = l + ((r - l) / 8) * 8;
			unsigned long long* inArrayCurr = (unsigned long long*)(inArray + current);
			for (; current < last_by_eight; current += 8, inArrayCurr++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
			{
				unsigned long long eight_bytes = *inArrayCurr;
				countLeft[ eight_bytes        & 0xff]++;
				countLeft[(eight_bytes >>  8) & 0xff]++;
				countLeft[(eight_bytes >> 16) & 0xff]++;
				countLeft[(eight_bytes >> 24) & 0xff]++;
				countLeft[(eight_bytes >> 32) & 0xff]++;
				countLeft[(eight_bytes >> 40) & 0xff]++;
				countLeft[(eight_bytes >> 48) & 0xff]++;
				countLeft[(eight_bytes >> 56) & 0xff]++;
			}
			for (; current < r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
				countLeft[inArray[current]]++;

			return countLeft;
		}

		size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;  // average without overflow

#if defined(USE_PPL)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { countLeft  = HistogramOneByteComponentParallel_4 <NumberOfBins>(inArray, l, m, parallelThreshold); },
			[&] { countRight = HistogramOneByteComponentParallel_4 <NumberOfBins>(inArray, m, r, parallelThreshold); }
		);
		// Combine left and right results
		for (size_t j = 0; j < NumberOfBins; j++)
			countLeft[j] += countRight[j];

		delete[] countRight;

		return countLeft;
	}


	// left (l) boundary is inclusive and right (r) boundary is exclusive
	template< unsigned NumberOfBins >
	inline size_t* HistogramOneByteComponentParallel_2(unsigned char inArray[], size_t l, size_t r, size_t parallelThreshold = 64 * 1024)
	{
		size_t* countLeft_0 = NULL;
		size_t* countLeft_1 = NULL;
		size_t* countLeft_2 = NULL;
		size_t* countLeft_3 = NULL;
		size_t* countRight  = NULL;

		if (l >= r)      // zero elements to compare
		{
			countLeft_0 = new size_t[NumberOfBins]{};
			return countLeft_0;
		}
		if ((r - l) <= parallelThreshold)
		{
			countLeft_0 = new size_t[NumberOfBins]{};
			countLeft_1 = new size_t[NumberOfBins]{};
			countLeft_2 = new size_t[NumberOfBins]{};
			countLeft_3 = new size_t[NumberOfBins]{};

			size_t last_by_four = l + ((r - l) / 4) * 4;
			size_t current = l;
			for (; current < last_by_four;)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
			{
				countLeft_0[inArray[current]]++;  current++;
				countLeft_1[inArray[current]]++;  current++;
				countLeft_2[inArray[current]]++;  current++;
				countLeft_3[inArray[current]]++;  current++;
			}
			for (; current < r; current++)    // possibly last element
				countLeft_0[inArray[current]]++;

			// Combine the two count arrays into a single arrray to return
			for (size_t count_index = 0; count_index < NumberOfBins; count_index++)
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

		size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;  // average without overflow

#if defined(USE_PPL)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { countLeft_0 = HistogramOneByteComponentParallel_2 <NumberOfBins>(inArray, l, m, parallelThreshold); },
			[&] { countRight = HistogramOneByteComponentParallel_2 <NumberOfBins>(inArray, m, r, parallelThreshold); }
		);
		// Combine left and right results
		for (size_t j = 0; j < NumberOfBins; j++)
			countLeft_0[j] += countRight[j];

		delete[] countRight;

		return countLeft_0;
	}

	// left (l) boundary is inclusive and right (r) boundary is exclusive
	template< unsigned NumberOfBins >
	inline size_t* HistogramOneByteComponentParallel_3(unsigned char inArray[], size_t l, size_t r, size_t parallelThreshold = 64 * 1024)
	{
		size_t* countLeft_0 = NULL;
		size_t* countRight  = NULL;

		if (l >= r)      // zero elements to compare
		{
			countLeft_0 = new size_t[NumberOfBins]{};
			return countLeft_0;
		}
		if ((r - l) <= parallelThreshold)
		{
			countLeft_0 = new size_t[NumberOfBins]{};
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
			__declspec(align(64)) size_t countLeft_1[NumberOfBins] = { 0 };
			__declspec(align(64)) size_t countLeft_2[NumberOfBins] = { 0 };
			__declspec(align(64)) size_t countLeft_3[NumberOfBins] = { 0 };
#else
			size_t countLeft_1[NumberOfBins] __attribute__((aligned(64))) = { 0 };
			size_t countLeft_2[NumberOfBins] __attribute__((aligned(64))) = { 0 };
			size_t countLeft_3[NumberOfBins] __attribute__((aligned(64))) = { 0 };
#endif

			size_t last_by_four = l + ((r - l) / 4) * 4;
			size_t current = l;
			for (; current < last_by_four;)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
			{
				countLeft_0[inArray[current]]++;  current++;
				countLeft_1[inArray[current]]++;  current++;
				countLeft_2[inArray[current]]++;  current++;
				countLeft_3[inArray[current]]++;  current++;
			}
			for (; current < r; current++)    // possibly last element
				countLeft_0[inArray[current]]++;

			// Combine the two count arrays into a single arrray to return
			for (size_t count_index = 0; count_index < NumberOfBins; count_index++)
			{
				countLeft_0[count_index] += countLeft_1[count_index];
				countLeft_0[count_index] += countLeft_2[count_index];
				countLeft_0[count_index] += countLeft_3[count_index];
			}

			return countLeft_0;
		}

		size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;  // average without overflow

#if defined(USE_PPL)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { countLeft_0 = HistogramOneByteComponentParallel_3 <NumberOfBins>(inArray, l, m, parallelThreshold); },
			[&] { countRight  = HistogramOneByteComponentParallel_3 <NumberOfBins>(inArray, m, r, parallelThreshold); }
		);
		// Combine left and right results
		for (size_t j = 0; j < NumberOfBins; j++)
			countLeft_0[j] += countRight[j];

		delete[] countRight;

		return countLeft_0;
	}

	template< unsigned PowerOfTwoRadix, unsigned Log2ofPowerOfTwoRadix >
	inline size_t** HistogramByteComponentsParallel(unsigned inArray[], size_t l, size_t r, size_t parallelThreshold = 64 * 1024)
	{
		const unsigned numberOfDigits = Log2ofPowerOfTwoRadix;
		const unsigned NumberOfBins   = PowerOfTwoRadix;

		size_t** countLeft  = NULL;
		size_t** countRight = NULL;

		if (l > r)      // zero elements to compare
		{
			countLeft = new size_t* [numberOfDigits];

			for (unsigned i = 0; i < numberOfDigits; i++)
			{
				countLeft[i] = new size_t[NumberOfBins];
				for (unsigned j = 0; j < NumberOfBins; j++)
					countLeft[i][j] = 0;
			}
			return countLeft;
		}
		if ((r - l + 1) <= parallelThreshold)
		{
			countLeft = new size_t* [numberOfDigits];

			for (unsigned i = 0; i < numberOfDigits; i++)
			{
				countLeft[i] = new size_t[NumberOfBins];
				for (unsigned j = 0; j < NumberOfBins; j++)
					countLeft[i][j] = 0;
			}
			// Faster version, since it doesn't use a 2-D array, reducing one level of indirection
			size_t* count0 = countLeft[0];
			size_t* count1 = countLeft[1];
			size_t* count2 = countLeft[2];
			size_t* count3 = countLeft[3];
#if 1
			for (size_t current = l; current <= r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
			{
				unsigned value = inArray[current];
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

		size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;   // average without overflow

#if defined(USE_PPL)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { countLeft  = HistogramByteComponentsParallel <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l,     m, parallelThreshold); },
			[&] { countRight = HistogramByteComponentsParallel <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, m + 1, r, parallelThreshold); }
		);
		// Combine left and right results
		for (unsigned i = 0; i < numberOfDigits; i++)
			for (unsigned j = 0; j < NumberOfBins; j++)
				countLeft[i][j] += countRight[i][j];

		for (unsigned i = 0; i < numberOfDigits; i++)
			delete[] countRight[i];
		delete[] countRight;

		return countLeft;
	}

	// Returns count[quanta][NumberOfBins]
	template< unsigned PowerOfTwoRadix, unsigned Log2ofPowerOfTwoRadix >
	inline size_t** HistogramByteComponentsAcrossWorkQuantasQC(unsigned inArray[], size_t l, size_t r, size_t workQuanta, size_t numberOfQuantas, unsigned whichByte)
	{
		const unsigned NumberOfBins = PowerOfTwoRadix;
		const unsigned mask = 0xff;
		int shiftRightAmount = (int)(8 * whichByte);
		//cout << "HistogramQC: l = " << l << "  r = " << r << "  workQuanta = " << workQuanta << "  quanta = " << quanta << "  whichByte = " << whichByte << endl;

		size_t** count = new size_t * [numberOfQuantas];
		for (size_t i = 0; i < numberOfQuantas; i++)
		{
			count[i] = new size_t[NumberOfBins];
			for (unsigned long j = 0; j < NumberOfBins; j++)
				count[i][j] = 0;
		}

		if (l > r)
			return count;

		size_t startQuanta = l / workQuanta;
		size_t endQuanta   = r / workQuanta;
		if (startQuanta == endQuanta)       // counting within a single workQuanta, either partial or full
		{
			size_t q = startQuanta;
			for (size_t currIndex = l; currIndex <= r; currIndex++)
			{
				unsigned inByte = (inArray[currIndex] >> shiftRightAmount) & mask;
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
				unsigned inByte = (inArray[currIndex] >> shiftRightAmount) & mask;
				count[q][inByte]++;
			}

			// process endQuanta, which is either partial or full
			q = endQuanta;
			for (currIndex = endQuanta * workQuanta; currIndex <= r; currIndex++)
			{
				unsigned inByte = (inArray[currIndex] >> shiftRightAmount) & mask;
				count[q][inByte]++;
			}

			// process full workQuantas > startQuanta and < endQuanta
			currIndex = (startQuanta + 1) * workQuanta;
			endQuanta--;
			for (q = startQuanta + 1; q <= endQuanta; q++)
			{
				for (size_t j = 0; j < workQuanta; j++)
				{
					unsigned inByte = (inArray[currIndex++] >> shiftRightAmount) & mask;
					count[q][inByte]++;
				}
			}
		}

		return count;
	}

	template< unsigned PowerOfTwoRadix, unsigned Log2ofPowerOfTwoRadix >
	inline size_t** HistogramByteComponentsQCParInner(unsigned inArray[], size_t l, size_t r, size_t workQuanta, size_t numberOfQuantas, unsigned whichByte, size_t parallelThreshold = 16 * 1024)
	{
		const unsigned NumberOfBins = PowerOfTwoRadix;
		size_t** countLeft  = NULL;
		size_t** countRight = NULL;

		if (l > r)      // zero elements to compare
		{
			size_t** countLeft = new size_t * [numberOfQuantas];
			for (size_t i = 0; i < numberOfQuantas; i++)
			{
				countLeft[i] = new size_t[NumberOfBins];
				for (unsigned j = 0; j < NumberOfBins; j++)
					countLeft[i][j] = 0;
			}
			return countLeft;
		}
		if ((r - l + 1) <= parallelThreshold)
			return HistogramByteComponentsAcrossWorkQuantasQC<PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l, r, workQuanta, numberOfQuantas, whichByte);

		size_t m = ((r + l) / 2);

#if defined(USE_PPL)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { countLeft  = HistogramByteComponentsQCParInner <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l,     m, workQuanta, numberOfQuantas, whichByte, parallelThreshold); },
			[&] { countRight = HistogramByteComponentsQCParInner <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, m + 1, r, workQuanta, numberOfQuantas, whichByte, parallelThreshold); }
		);
		// Combine left and right results (reduce step), only for workQuantas for which the counts were computed
		size_t startQuanta = l / workQuanta;
		size_t endQuanta   = r / workQuanta;
		for (size_t i = startQuanta; i <= endQuanta; i++)
			for (unsigned long j = 0; j < NumberOfBins; j++)
				countLeft[i][j] += countRight[i][j];

		for (size_t i = 0; i < numberOfQuantas; i++)
			delete[] countRight[i];
		delete[] countRight;

		return countLeft;
	}

	template< unsigned PowerOfTwoRadix, unsigned Log2ofPowerOfTwoRadix >
	inline size_t** HistogramByteComponentsQCPar(unsigned* inArray, size_t l, size_t r, size_t workQuanta, size_t numberOfQuantas, unsigned whichByte, size_t parallelThreshold = 16 * 1024)
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

	// This version did not seem to speed up over the single count array version. It proves that Histogram is not the bottleneck.
	template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
	inline size_t* HistogramOneByteComponentParallel_2(unsigned inArray[], size_t l, size_t r, unsigned long shiftRight, size_t parallelThreshold = 64 * 1024)
	{
		const unsigned long NumberOfBins = PowerOfTwoRadix;

		size_t* countLeft_0 = NULL;
		size_t* countLeft_1 = NULL;
		size_t* countLeft_2 = NULL;
		size_t* countLeft_3 = NULL;
		size_t* countRight  = NULL;

		if (l > r)      // zero elements to compare
		{
			countLeft_0 = new size_t[NumberOfBins]{};
			return countLeft_0;
		}
		if ((r - l + 1) <= parallelThreshold)
		{
			countLeft_0 = new size_t[NumberOfBins]{};
			countLeft_1 = new size_t[NumberOfBins]{};
			countLeft_2 = new size_t[NumberOfBins]{};
			countLeft_3 = new size_t[NumberOfBins]{};

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
			for (size_t count_index = 0; count_index < NumberOfBins; count_index++)
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

#if defined(USE_PPL)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { countLeft_0 = HistogramOneByteComponentParallel_2 <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l,     m, shiftRight, parallelThreshold); },
			[&] { countRight  = HistogramOneByteComponentParallel_2 <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, m + 1, r, shiftRight, parallelThreshold); }
		);
		// Combine left and right results
		for (size_t j = 0; j < NumberOfBins; j++)
			countLeft_0[j] += countRight[j];

		delete[] countRight;

		return countLeft_0;
	}

	template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
	inline size_t* HistogramOneByteComponentParallel(unsigned inArray[], size_t l, size_t r, unsigned long shiftRight, size_t parallelThreshold = 64 * 1024)
	{
		const size_t NumberOfBins = PowerOfTwoRadix;
		const unsigned mask = NumberOfBins - 1;

		size_t* countLeft  = NULL;
		size_t* countRight = NULL;

		if (l > r)      // zero elements to compare
		{
			countLeft = new size_t[NumberOfBins]{};
			return countLeft;
		}
		if ((r - l + 1) <= parallelThreshold)
		{
			countLeft = new size_t[NumberOfBins]{};

			for (size_t current = l; current <= r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
				countLeft[(inArray[current] >> shiftRight) & mask]++;

			return countLeft;
		}

		size_t m = r / 2 + l / 2 + (r % 2 + l % 2) / 2;

#if defined(USE_PPL)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { countLeft  = HistogramOneByteComponentParallel <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, l, m, shiftRight, parallelThreshold); },
			[&] { countRight = HistogramOneByteComponentParallel <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(inArray, m + 1, r, shiftRight, parallelThreshold); }
		);
		// Combine left and right results
		for (size_t j = 0; j < NumberOfBins; j++)
			countLeft[j] += countRight[j];

		delete[] countRight;

		return countLeft;
	}

}