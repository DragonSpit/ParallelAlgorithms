#ifndef _RadixPartition_h
#define _RadixPartition_h

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

// Simplified the implementation of the inner loop.
template< class _Type, unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, long Threshold >
inline void PartitionRadixMsdUIntInner(_Type* a, size_t start, size_t a_size, int shiftRightAmount, size_t* k, size_t kStart, size_t kLength)
{
	//printf("PartitionRadixMsdUIntInner: a_size=%zu, shiftRightAmount=%d, kStart=%zu, kLength=%zu  kValue=%zu\n", a_size, shiftRightAmount, kStart, kLength, k[kStart]);
    size_t last = start + a_size;
    const unsigned long BitMask = PowerOfTwoRadix - 1;
    size_t count[PowerOfTwoRadix];

    for (unsigned long i = 0; i < PowerOfTwoRadix; i++)     count[i] = 0;
    for (size_t _current = start; _current < last; _current++)	    // Scan the array and count the number of times each value appears
        count[(unsigned)((a[_current] >> shiftRightAmount) & BitMask)]++;

    size_t startOfBin[PowerOfTwoRadix + 1], endOfBin[PowerOfTwoRadix], nextBin = 1;
    startOfBin[0] = endOfBin[0] = start;    startOfBin[PowerOfTwoRadix] = start;			// sentinal
    for (size_t i = 1; i < PowerOfTwoRadix; i++)
		startOfBin[i] = endOfBin[i] = startOfBin[i - 1] + count[i - 1]; // endOfBin is exclusive

    for (size_t _current = start; _current < last;)
    {
        unsigned digit;
        _Type _current_element = a[_current];	// get the compiler to recognize that a register can be used for the loop instead of a[_current] memory location
        while (endOfBin[digit = (unsigned)((_current_element >> shiftRightAmount) & BitMask)] != _current)  _swap(_current_element, a[endOfBin[digit]++]);
        a[_current] = _current_element;

        endOfBin[digit]++;
        while (endOfBin[nextBin - 1] == startOfBin[nextBin])  nextBin++;	// skip over empty and full bins, when the end of the current bin reaches the start of the next bin
        _current = endOfBin[nextBin - 1];
    }
    if (shiftRightAmount > 0)          // end recursion when all the bits have been processes
    {
        if (shiftRightAmount >= Log2ofPowerOfTwoRadix)	shiftRightAmount -= Log2ofPowerOfTwoRadix;
        else											shiftRightAmount = 0;

        for (size_t bin = 0, kIndex = kStart; bin < PowerOfTwoRadix && kIndex < (kStart + kLength); bin++)
        {
			//printf("PartitionRadixMsdUIntInner: bin=%zu, startOfBin=%zu, endOfBin=%zu, kIndex=%zu, kValue=%zu\n", bin, startOfBin[bin], endOfBin[bin], kIndex, k[kIndex]);
            // Recurse only into a bin which contains one or more of the k[] elements
            if (startOfBin[bin] < endOfBin[bin] && k[kIndex] >= startOfBin[bin] && k[kIndex] < endOfBin[bin])
            {
                size_t kNewStart = kIndex++;
                size_t kNewLength = 1; // at least one of the k[] elements is in this bin. Determine if more k[] element are in this bin, and pass them into the recursive call for this bin.
                for (; kIndex < (kStart + kLength); kIndex++)
                {
                    if (k[kIndex] >= startOfBin[bin] && k[kIndex] < endOfBin[bin])
                        kNewLength++;
                    else break;
                }
                size_t numberOfElements = endOfBin[bin] - startOfBin[bin];
                if (numberOfElements >= Threshold)		// endOfBin is exclusive
                    PartitionRadixMsdUIntInner< _Type, PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(a, startOfBin[bin], numberOfElements, shiftRightAmount, k, kNewStart, kNewLength);
                else
                    insertionSortSimilarToSTLnoSelfAssignment(&a[startOfBin[bin]], numberOfElements);
            }
        }
    }
}

/**
 * @brief In-place Radix Partition by the k-th element in an array.
 * @param ArrayToBePartitioned array that is to be partitioned in place
 * @param aLength Length of the array or subarray
 * @param k Index of the desired element to be selected
 * @return the k-th element in the array
 */
inline unsigned PartitionRadix(unsigned arrayToBePartitioned[], size_t aLength, size_t k)
{
    //if (arrayToBeSelected == null)
    //    throw new ArgumentNullException(nameof(arrayToBeSelected));
    //if (arrayToBeSelected.Length <= 0)
    //    throw new ArgumentOutOfRangeException(nameof(arrayToBeSelected.Length), "array length is invalid");
    //if (k < 0 || k > arrayToBeSelected.Length)
    //    throw new ArgumentOutOfRangeException(nameof(k), "k must be between start and (start + length)");
    const long PowerOfTwoRadix = 256;
    const long Log2ofPowerOfTwoRadix = 8;
    const long Threshold = 48;
    const unsigned BitsPerDigit = 8;

    int shiftRightAmount = (sizeof(unsigned) * 8) - BitsPerDigit;
    size_t kArray[1] = { k };
    size_t kStart = 0;
	size_t kLength = 1;
    PartitionRadixMsdUIntInner< unsigned, PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(arrayToBePartitioned, 0, aLength, shiftRightAmount, kArray, kStart, kLength);
    return arrayToBePartitioned[k];
}
/**
 * @brief In-place Radix Partition by the k-th element in an array. 
 * @param arrayToBePartitioned Array that is to be partitioned in place
 * @param start Starting index of the subarray
 * @param aLength Length of the subarray
 * @param k Index of the desired element to be selected
 * @return the k-th element in the array
 */
inline unsigned PartitionRadix(unsigned arrayToBePartitioned[], size_t aStart, size_t aLength, size_t k)
{
    //if (arrayToBePartitioned == null)
    //    throw new ArgumentNullException(nameof(arrayToBePartitioned));
    //if (start < 0 || aLength <= 0)
    //    throw new ArgumentOutOfRangeException(nameof(k), "l or r are invalid");
    //if (k < start || k >= start + aLength)
    //    throw new ArgumentOutOfRangeException(nameof(k), "k must be between start and (start + aLength)");
    const long PowerOfTwoRadix = 256;
    const long Log2ofPowerOfTwoRadix = 8;
    const long Threshold = 48;
    const unsigned BitsPerDigit = 8;

    int shiftRightAmount = (sizeof(unsigned) * 8) - BitsPerDigit;
    size_t kArray[1] = { k };
    size_t kStart = 0;
    size_t kLength = 1;
    PartitionRadixMsdUIntInner< unsigned, PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(arrayToBePartitioned, aStart, aLength, shiftRightAmount, kArray, kStart, kLength);
    return arrayToBePartitioned[k];
}
/**
 * @brief In-place Radix Partition by the k-th array elements in an array.
 * @param ArrayToBeSelected array that is to be sorted in place
 * @param k Array of indices of the desired elements to be selected
 */
inline void PartitionRadix(unsigned arrayToBePartitioned[], size_t aLength, size_t k[], size_t kLength)
{
    //if (arrayToBePartitioned == null)
    //    throw new ArgumentNullException(nameof(arrayToBePartitioned));
    //if (arrayToBePartitioned.Length <= 0)
    //    throw new ArgumentOutOfRangeException(nameof(arrayToBePartitioned.Length), "array length is invalid");
    //if (k < 0 || k >= arrayToBePartitioned.Length)
    //    throw new ArgumentOutOfRangeException(nameof(k), "k must be between 0 and (arrayToBePartitioned.Length - 1)");
    const long PowerOfTwoRadix = 256;
    const long Log2ofPowerOfTwoRadix = 8;
    const long Threshold = 48;
    const unsigned BitsPerDigit = 8;

    int shiftRightAmount = (sizeof(unsigned) * 8) - BitsPerDigit;
    PartitionRadixMsdUIntInner< unsigned, PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(arrayToBePartitioned, 0, aLength, shiftRightAmount, k, 0, kLength);
}
/**
 * @brief In-place Radix Partition by the k-th array elements in an array.
 * @param arrayToBePartitioned Array that is to be partitioned in place
 * @param aStart Starting index of the subarray
 * @param aLength Length of the subarray
 * @param k Array of indices of the desired elements to be selected
 * @param kStart Starting index of the k array
 * @param kLength Length of the k array
 */
inline void PartitionRadix(unsigned arrayToBePartitioned[], size_t aStart, size_t aLength, size_t k[], size_t kStart, size_t kLength)
{
    //if (arrayToBePartitioned == null)
    //    throw new ArgumentNullException(nameof(arrayToBePartitioned));
    //if (aStart < 0 || aLength <= 0)
    //    throw new ArgumentOutOfRangeException(nameof(k), "l or r are invalid");
    //if (k < aStart || k >= aStart + aLength)
    //    throw new ArgumentOutOfRangeException(nameof(k), "k must be between aStart and (aStart + aLength)");
    const long PowerOfTwoRadix = 256;
    const long Log2ofPowerOfTwoRadix = 8;
    const long Threshold = 48;
    const unsigned BitsPerDigit = 8;

    int shiftRightAmount = (sizeof(unsigned) * 8) - BitsPerDigit;
    PartitionRadixMsdUIntInner< unsigned, PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(arrayToBePartitioned, aStart, aLength, shiftRightAmount, k, kStart, kLength);
}

#endif