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

