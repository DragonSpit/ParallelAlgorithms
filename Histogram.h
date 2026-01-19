// TODO: Switch histogram calculation from mask/shift to union

#pragma once

template< unsigned PowerOfTwoRadix, unsigned Log2ofPowerOfTwoRadix >
inline size_t* HistogramByteComponents_1(unsigned inArray[], size_t l, size_t r)
{
	const unsigned numberOfDigits = Log2ofPowerOfTwoRadix;
	const unsigned NumberOfBins   = PowerOfTwoRadix;

	size_t* count = new size_t[numberOfDigits * NumberOfBins]{};

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
