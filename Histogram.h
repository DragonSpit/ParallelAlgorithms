// TODO: Switch histogram calculation from mask/shift to union

#pragma once

inline size_t* HistogramByteComponents(unsigned inArray[], size_t l, size_t r)
{
	const unsigned BitsPerDigit   = 8;
	const unsigned NumberOfBins   = 1 << BitsPerDigit;
	const unsigned NumberOfDigits = (sizeof(unsigned) * 8 + BitsPerDigit - 1) / BitsPerDigit;

	size_t* count = new size_t[NumberOfDigits * NumberOfBins]{};

	size_t* count0 = count + (0 * NumberOfBins);
	size_t* count1 = count + (1 * NumberOfBins);
	size_t* count2 = count + (2 * NumberOfBins);
	size_t* count3 = count + (3 * NumberOfBins);

	for (size_t current = l; current <= r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
	{
		unsigned value = inArray[current];
		count0[ value        & 0xff]++;
		count1[(value >>  8) & 0xff]++;
		count2[(value >> 16) & 0xff]++;
		count3[(value >> 24) & 0xff]++;
	}
	return count;
}

inline size_t* HistogramOneByteComponent(unsigned inArray[], size_t l, size_t r, unsigned shiftAmount)
{
	const unsigned BitsPerDigit = 8;
	const unsigned NumberOfBins = 1 << BitsPerDigit;

	size_t* count = new size_t[NumberOfBins]{};

	for (size_t current = l; current < r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
		count[(inArray[current] >> shiftAmount) & 0xff]++;
	return count;
}

inline size_t* HistogramOneByteComponent(unsigned inArray[], size_t l, size_t r, unsigned shiftAmount, size_t* count)
{
	const unsigned BitsPerDigit = 8;
	const unsigned NumberOfBins = 1 << BitsPerDigit;

	for (int i = 0; i < NumberOfBins; i++)
		count[i] = 0;
	for (size_t current = l; current < r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
		count[(inArray[current] >> shiftAmount) & 0xff]++;
	return count;
}

inline size_t* HistogramOneComponent(unsigned inArray[], size_t l, size_t r, unsigned shiftAmount, unsigned bitsPerDigit, size_t* count)
{
	const unsigned NumberOfBins = 1 << bitsPerDigit;
	const unsigned Mask = NumberOfBins - 1;

	for (unsigned i = 0; i < NumberOfBins; i++)
		count[i] = 0;
	for (size_t current = l; current < r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
		count[(inArray[current] >> shiftAmount) & Mask]++;
	return count;
}

// Optimized by realizing that incrementing the same memory location, in the case of constant inArray, is a loop dependency with memory access within the dependency.
// Nearly2X faster than the non-optimized version for constant and pre-sorted arrays, but is slower for random arrays.
inline size_t* HistogramOneByteComponentOpt(unsigned inArray[], size_t l, size_t r, unsigned shiftAmount)
{
	const unsigned BitsPerDigit = 8;
	const unsigned NumberOfBins = 1 << BitsPerDigit;

	size_t* count_0 = new size_t[NumberOfBins]{};
	size_t* count_1 = new size_t[NumberOfBins]{};  // extra count arrays
	size_t* count_2 = new size_t[NumberOfBins]{};
	size_t* count_3 = new size_t[NumberOfBins]{};

	size_t current;
	size_t last_by_four = l + ((r - l) / 4) * 4;
	for (current = l; current < last_by_four;)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
	{
		count_0[(inArray[current] >> shiftAmount) & 0xff]++; current++;
		count_1[(inArray[current] >> shiftAmount) & 0xff]++; current++;
		count_2[(inArray[current] >> shiftAmount) & 0xff]++; current++;
		count_3[(inArray[current] >> shiftAmount) & 0xff]++; current++;
	}

	for (; current < r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
		count_0[(inArray[current] >> shiftAmount) & 0xff]++;

	// Combine the counts from the extra count arrays into the main count array
	for (size_t i = 0; i < NumberOfBins; i++)
		count_0[i] += count_1[i] + count_2[i] + count_3[i];

	delete[] count_1;
	delete[] count_2;
	delete[] count_3;

	return count_0;
}

// l is inclusing and r is exclusive
// Nearly 2X faster than the version with a single additional count array for constant and pre-sorted arrays, but is slower for random arrays.
inline size_t* HistogramOneComponentOpt(unsigned inArray[], size_t l, size_t r, unsigned shiftAmount, unsigned bitsPerDigit, size_t* count)
{
	const unsigned NumberOfBins = 1 << bitsPerDigit;
	const unsigned Mask = NumberOfBins - 1;

	size_t* count_all = new size_t[4 * NumberOfBins]{};  // extra count arrays
	size_t* count_0 = count_all + (0 * NumberOfBins);
	size_t* count_1 = count_all + (1 * NumberOfBins);
	size_t* count_2 = count_all + (2 * NumberOfBins);

	if (l < r)
	{
		size_t current;
		size_t last_by_three = l + ((r - l) / 3) * 3;
		for (current = l; current < last_by_three;)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
		{
			count_0[(inArray[current] >> shiftAmount) & Mask]++; current++;
			count_1[(inArray[current] >> shiftAmount) & Mask]++; current++;
			count_2[(inArray[current] >> shiftAmount) & Mask]++; current++;
		}

		for (; current < r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
			count_0[(inArray[current] >> shiftAmount) & Mask]++;

		// Combine the counts from the extra count arrays into the main count array
		for (size_t i = 0; i < NumberOfBins; i++)
			count[i] = count_0[i] + count_1[i] + count_2[i];
	}
	delete[] count_all;
	return count;
}

inline size_t* HistogramWordComponents(unsigned inArray[], size_t l, size_t r)
{
	const unsigned BitsPerDigit = 16;
	const unsigned NumberOfBins = 1 << BitsPerDigit;
	const unsigned NumberOfDigits = (sizeof(unsigned) * 8 + BitsPerDigit - 1) / BitsPerDigit;

	size_t* count = new size_t[NumberOfDigits * NumberOfBins]{};

	size_t* count0 = count + (0 * NumberOfBins);
	size_t* count1 = count + (1 * NumberOfBins);

	for (size_t current = l; current <= r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
	{
		unsigned value = inArray[current];
		count0[value         & 0xffff]++;
		count1[(value >> 16) & 0xffff]++;
	}
	return count;
}

inline size_t* HistogramOneWordComponent(unsigned inArray[], size_t l, size_t r, unsigned shiftAmount)
{
	const unsigned BitsPerDigit = 16;
	const unsigned NumberOfBins = 1 << BitsPerDigit;

	size_t* count = new size_t[NumberOfBins]{};

	for (size_t current = l; current <= r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
		count[(inArray[current] >> shiftAmount) & 0xffff]++;
	return count;
}

inline void HistogramByteSingleComponent(unsigned inArray[], size_t l, size_t r, unsigned shiftAmount, size_t* count)
{
	const size_t NumberOfBins = 256;
	for (size_t i = 0; i < NumberOfBins; i++)
		count[i] = 0;
	for (size_t current = l; current <= r; current++)
		count[(inArray[current] >> shiftAmount) & 0xff]++;
}


inline size_t* HistogramNbitComponents(unsigned inArray[], size_t l, size_t r, unsigned bitsPerComponent)
{
	if (inArray == NULL)
		return NULL;
	const int NumBitsInUInt = sizeof(unsigned) * 8;
	if (bitsPerComponent > NumBitsInUInt || bitsPerComponent == 0)
		return NULL;

	size_t numberOfBins = (size_t)1 << bitsPerComponent;
	unsigned numberOfDigits = (NumBitsInUInt + bitsPerComponent - 1) / bitsPerComponent;  // ceiling division
	size_t* count = new size_t[numberOfDigits * numberOfBins]{};
	unsigned bitMask = (unsigned)(numberOfBins - 1);

	for (size_t current = l; current <= r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
	{
		unsigned value = inArray[current];
		for (unsigned d = 0; d < numberOfDigits; d++)
			count[d * numberOfBins + ((value >> (d * bitsPerComponent)) & bitMask)]++;
	}
	return count;
}

template< unsigned PowerOfTwoRadix, unsigned Log2ofPowerOfTwoRadix >
inline size_t** HistogramByteComponents(unsigned long long inArray[], int l, int r)
{
	const unsigned numberOfDigits = Log2ofPowerOfTwoRadix;
	const unsigned NumberOfBins   = PowerOfTwoRadix;

	size_t** count = new size_t * [numberOfDigits];

	for (unsigned i = 0; i < numberOfDigits; i++)
		count[i] = new size_t[NumberOfBins]{};

	// Faster version, since it doesn't use a 2-D array, reducing one level of indirection
	size_t* count0 = count[0];
	size_t* count1 = count[1];
	size_t* count2 = count[2];
	size_t* count3 = count[3];

	for (int current = l; current <= r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
	{
		unsigned long value = inArray[current];
		count0[value         & 0xff]++;
		count1[(value >>  8) & 0xff]++;
		count2[(value >> 16) & 0xff]++;
		count3[(value >> 24) & 0xff]++;
	}
	return count;
}

