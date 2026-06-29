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
    size_t last = start + a_size - 1;
    size_t count[PowerOfTwoRadix];
    const unsigned long BitMask = PowerOfTwoRadix - 1;
    for (unsigned long i = 0; i < PowerOfTwoRadix; i++)     count[i] = 0;
    for (size_t _current = start; _current <= last; _current++)	    // Scan the array and count the number of times each value appears
        count[(unsigned)((a[_current] >> shiftRightAmount) & BitMask)]++;

    size_t startOfBin[PowerOfTwoRadix + 1], endOfBin[PowerOfTwoRadix], nextBin = 1;
    startOfBin[0] = endOfBin[0] = start;    startOfBin[PowerOfTwoRadix] = start;			// sentinal
    for (size_t i = 1; i < PowerOfTwoRadix; i++)
		startOfBin[i] = endOfBin[i] = startOfBin[i - 1] + count[i - 1]; // endOfBin is exclusive

    for (size_t _current = start; _current <= last;)
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
 * @param ArrayToBeSelected array that is to be sorted in place
 * @param k Index of the desired element to be selected
 * @return the k-th element in the array
 */
inline unsigned PartitionRadix(unsigned arrayToBeSelected[], size_t length, size_t k)
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
    PartitionRadixMsdUIntInner< unsigned, PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(arrayToBeSelected, 0, length, shiftRightAmount, kArray, 0, 1);
    return arrayToBeSelected[k];
}
#if 0
/**
 * @brief In-place Radix Partition by the k-th element in an array. 
 * @param arrayToBeSelected Array that is to be partitioned in place
 * @param start Starting index of the subarray
 * @param length Length of the subarray
 * @param k Index of the desired element to be selected
 * @return the k-th element in the array
 */
inline unsigned PartitionRadix(unsigned arrayToBeSelected[], size_t start, size_t length, size_t k)
{
    //if (arrayToBeSelected == null)
    //    throw new ArgumentNullException(nameof(arrayToBeSelected));
    //if (start < 0 || length <= 0)
    //    throw new ArgumentOutOfRangeException(nameof(k), "l or r are invalid");
    //if (k < start || k >(start + arrayToBeSelected.Length))
    //    throw new ArgumentOutOfRangeException(nameof(k), "k must be between start and (start + length)");
    const unsigned BitsPerDigit = 8;
    int shiftRightAmount = (sizeof(unsigned) * 8) - BitsPerDigit;
	size_t kArray[1] = { k };
    PartitionRadixMsdUIntInner(arrayToBeSelected, start, length, kArray, 0, 1, shiftRightAmount, BitsPerDigit);
    return arrayToBeSelected[k];
}
#endif
#if 0
/**
 * @brief In-place Radix Partition by the k-th array elements in an array.
 * @param arrayToBeSelected Array that is to be partitioned in place
 * @param start Starting index of the subarray
 * @param length Length of the subarray
 * @param k Array of indices of the desired elements to be selected
 */
inline void PartitionRadix(unsigned arrayToBeSelected[], size_t start, size_t length, size_t k[], size_t kLength)
{
    //if (arrayToBeSelected == null)
    //    throw new ArgumentNullException(nameof(arrayToBeSelected));
    //if (start < 0 || length <= 0)
    //    throw new ArgumentOutOfRangeException(nameof(k), "l or r are invalid");
    //if (k < start || k >(start + arrayToBeSelected.Length))
    //    throw new ArgumentOutOfRangeException(nameof(k), "k must be between start and (start + length)");
    const unsigned BitsPerDigit = 8;
    int shiftRightAmount = (sizeof(unsigned) * 8) - BitsPerDigit;
    PartitionRadixMsdUIntInner(arrayToBeSelected, start, length, k, 0, kLength, shiftRightAmount, BitsPerDigit);
}
/**
 * @brief In-place Radix Partition by the k-th array elements in an array.
 * @param ArrayToBeSelected array that is to be sorted in place
 * @param k Array of indices of the desired elements to be selected
 */
inline void PartitionRadix(unsigned arrayToBeSelected[], size_t length, size_t k[], size_t kLength)
{
    //if (arrayToBeSelected == null)
    //    throw new ArgumentNullException(nameof(arrayToBeSelected));
    //if (arrayToBeSelected.Length <= 0)
    //    throw new ArgumentOutOfRangeException(nameof(arrayToBeSelected.Length), "array length is invalid");
    //if (k < 0 || k > arrayToBeSelected.Length)
    //    throw new ArgumentOutOfRangeException(nameof(k), "k must be between start and (start + length)");
    const unsigned BitsPerDigit = 8;
    int shiftRightAmount = (sizeof(unsigned) * 8) - BitsPerDigit;
    PartitionRadixMsdUIntInner(arrayToBeSelected, 0, length, k, 0, kLength, shiftRightAmount, BitsPerDigit);
}
#endif

#if 0
inline static void PartitionRadixMsdUIntInner(unsigned a[], size_t start, size_t length, size_t k[], size_t kStart, size_t kLength, unsigned shiftRightAmount, unsigned bitsPerDigit, int threshold = 50)
{
    if (length < threshold)
    {
        insertionSortSimilarToSTLnoSelfAssignment(&a[start], length);
        return;
    }
    size_t last = start + length;
    const unsigned NumberOfBins = 1 << bitsPerDigit;
    const unsigned bitMask = NumberOfBins - 1;

    size_t* count = new size_t[NumberOfBins];
    HistogramOneComponent(a, start, last, shiftRightAmount, bitsPerDigit, count);

    size_t* startOfBin = new size_t[NumberOfBins];
    size_t* endOfBin = new size_t[NumberOfBins];
    int nextBin = 1;
    startOfBin[0] = endOfBin[0] = start;
    for (unsigned i = 1; i < NumberOfBins; i++)
        startOfBin[i] = endOfBin[i] = startOfBin[i - 1] + count[i - 1];

    for (size_t _current = start; _current < last;)
    {
        unsigned digit;
        unsigned current_element = a[_current];  // get the compiler to recognize that a register can be used for the loop instead of a[_current] memory location
        while (endOfBin[digit = (current_element >> shiftRightAmount) & bitMask] != _current)
            (a[endOfBin[digit]], current_element) = (current_element, a[endOfBin[digit]++]);
        a[_current] = current_element;

        endOfBin[digit]++;
        while (endOfBin[nextBin - 1] == startOfBin[nextBin]) nextBin++;   // skip over empty and full bins, when the end of the current bin reaches the start of the next bin
        _current = endOfBin[nextBin - 1];
    }
    if (shiftRightAmount > 0)          // end recursion when all the bits have been processes
    {
        if (shiftRightAmount >= bitsPerDigit) shiftRightAmount -= bitsPerDigit;
        else shiftRightAmount = 0;

        for (size_t bin = 0, kIndex = kStart; bin < NumberOfBins && kIndex < (kStart + kLength); bin++)
        {
            // Recurse only into a bin which contains one or more of the k[] elements
            if (startOfBin[bin] <= endOfBin[bin] && k[kIndex] >= startOfBin[bin] && k[kIndex] < endOfBin[bin])
            {
                size_t kNewStart = kIndex++;
                size_t kNewLength = 1; // at least one of the k[] elements is in this bin. Determine if more k[] element are in this bin, and pass them into the recursive call for this bin.
                for (; kIndex < (kStart + kLength); kIndex++)
                {
                    if (k[kIndex] >= startOfBin[bin] && k[kIndex] < endOfBin[bin])
                        kNewLength++;
                    else break;
                }
                PartitionRadixMsdUIntInner(a, startOfBin[bin], endOfBin[bin] - startOfBin[bin], k, kNewStart, kNewLength, shiftRightAmount, bitsPerDigit, threshold);
            }
        }
    }
    delete[] endOfBin;
    delete[] startOfBin;
    delete[] count;
}
#endif

#endif