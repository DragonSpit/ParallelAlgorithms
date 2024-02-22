
#ifndef _RadixSortMSD_h
#define _RadixSortMSD_h

#include "RadixSortCommon.h"
#include "InsertionSort.h"

// Swap that does not check for self-assignment.
template< class _Type >
inline void _swap(_Type& a, _Type& b)
{
	_Type tmp = a;
	a = b;
	b = tmp;
}

// Simplified the implementation of the inner loop.
template< class _Type, unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, long Threshold >
inline void _RadixSort_Unsigned_PowerOf2Radix_L1(_Type* a, size_t a_size, _Type bitMask, unsigned long shiftRightAmount)
{
	size_t last = a_size - 1;
	size_t count[PowerOfTwoRadix];
	for (unsigned long i = 0; i < PowerOfTwoRadix; i++)     count[i] = 0;
	for (size_t _current = 0; _current <= last; _current++)	    // Scan the array and count the number of times each value appears
		count[(unsigned)((a[_current] & bitMask) >> shiftRightAmount)]++;

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
		else												shiftRightAmount = 0;

		for (unsigned long i = 0; i < PowerOfTwoRadix; i++)
		{
			size_t numberOfElements = endOfBin[i] - startOfBin[i];
			if (numberOfElements >= Threshold)		// endOfBin actually points to one beyond the bin
				_RadixSort_Unsigned_PowerOf2Radix_L1< _Type, PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(&a[startOfBin[i]], numberOfElements, bitMask, shiftRightAmount);
			else if (numberOfElements >= 2)
				insertionSortSimilarToSTLnoSelfAssignment(&a[startOfBin[i]], numberOfElements);
		}
	}
}

inline void hybrid_inplace_msd_radix_sort(unsigned* a, size_t a_size)
{
	if (a_size < 2)	return;

	const long PowerOfTwoRadix = 256;
	const long Log2ofPowerOfTwoRadix = 8;
	const long Threshold = 48;

	unsigned bitMask = 0x80000000;	// bitMask controls how many bits we process at a time
	unsigned long shiftRightAmount = 31;

	for (unsigned long i = 2; i < PowerOfTwoRadix; )	// if not power-of-two value then it will do up to the largest power-of-two value
	{													// that's smaller than the value provided (e.g. radix-10 will do radix-8)
		bitMask |= (bitMask >> 1);
		shiftRightAmount -= 1;
		i <<= 1;
	}

	if (a_size >= Threshold)
		_RadixSort_Unsigned_PowerOf2Radix_L1< unsigned, PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(a, a_size, bitMask, shiftRightAmount);
	else
		insertionSortSimilarToSTLnoSelfAssignment( a, a_size );
		//insertionSortHybrid(a, a_size);
}

template< unsigned PowerOfTwoRadix, unsigned Log2ofPowerOfTwoRadix, size_t Threshold, class _Type >
inline void _RadixSort_StableUnsigned_PowerOf2Radix_2(_Type* a, _Type* b, size_t last, _Type bitMask, unsigned shiftRightAmount, bool inputArrayIsDestination)
{
	const unsigned NumberOfBins = PowerOfTwoRadix;
	size_t count[NumberOfBins] = {};

	for (size_t _current = 0; _current <= last; _current++)				// Scan the array and count the number of times each value appears
		count[extractDigit(a[_current], bitMask, shiftRightAmount)]++;

	size_t startOfBin[NumberOfBins], endOfBin[NumberOfBins];
	startOfBin[0] = endOfBin[0] = 0;
	for (unsigned i = 1; i < NumberOfBins; i++)
		startOfBin[i] = endOfBin[i] = startOfBin[i - 1] + count[i - 1];

	for (size_t _current = 0; _current <= last; _current++)				// permute array elements
		b[endOfBin[extractDigit(a[_current], bitMask, shiftRightAmount)]++] = a[_current];

	bitMask >>= Log2ofPowerOfTwoRadix;
	if (bitMask != 0)						// end recursion when all the bits have been processes
	{
		if (shiftRightAmount >= Log2ofPowerOfTwoRadix)	shiftRightAmount -= Log2ofPowerOfTwoRadix;
		else											shiftRightAmount = 0;
		inputArrayIsDestination = !inputArrayIsDestination;
		for (unsigned i = 0; i < NumberOfBins; i++)
		{
			size_t numOfElements = endOfBin[i] - startOfBin[i];
			if (numOfElements >= Threshold)
				_RadixSort_StableUnsigned_PowerOf2Radix_2< PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(&b[startOfBin[i]], &a[startOfBin[i]], numOfElements - 1, bitMask, shiftRightAmount, inputArrayIsDestination);
			else {
				insertionSortSimilarToSTLnoSelfAssignment(&b[startOfBin[i]], numOfElements);
				if (inputArrayIsDestination)
					for (size_t j = startOfBin[i]; j < endOfBin[i]; j++)	// copy from external array back into the input array
						a[j] = b[j];
			}
		}
	}
	else {	// Done with recursion copy all of the bins
		if (!inputArrayIsDestination)
			for (size_t _current = 0; _current <= last; _current++)	// copy from external array back into the input array
				a[_current] = b[_current];
	}
}

template< class _Type >
inline void RadixSortMSDStablePowerOf2Radix_unsigned(_Type* a, _Type* b, size_t a_size)
{
	const size_t Threshold = 100;			// Threshold of when to switch to using Insertion Sort
	const unsigned PowerOfTwoRadix = 256;
	const unsigned Log2ofPowerOfTwoRadix = 8;
	// Create bit-mask and shift right amount
	unsigned shiftRightAmount = sizeof(_Type) * 8 - Log2ofPowerOfTwoRadix;
	_Type bitMask = (_Type)(((_Type)(PowerOfTwoRadix - 1)) << shiftRightAmount);	// bitMask controls/selects how many and which bits we process at a time

	// The beauty of using template arguments instead of function parameters for the Threshold and Log2ofPowerOfTwoRadix is
	// they are not pushed on the stack and are treated as constants, but local.
	if (a_size >= Threshold)
		_RadixSort_StableUnsigned_PowerOf2Radix_2< PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(a, b, a_size - 1, bitMask, shiftRightAmount, false);
	else
		insertionSortSimilarToSTLnoSelfAssignment(a, a_size);
}

#endif