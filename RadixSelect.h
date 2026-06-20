// TODO: Fix SelectRadixWord, as there seems to be an error, while the byte version works correctly.
// TODO: Apply the same optimization for the case of constant arrays as was done for Radix Sort.
#ifndef _RadixSelect_h
#define _RadixSelect_h

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

//const int PowerOfTwoRadix = 256;
const int Log2ofPowerOfTwoRadix = 8;

// Move elements outside the k-th bin, the bin that k is in, which belong to the k-th bin, into the k-th bin.
// Generic implementation that work for regions to the left or to the right of the k-th bin, and for any digit size.
inline static size_t MoveOutsideOfKthBinIn(unsigned a[], size_t startOfOb, size_t lengthOfOb, size_t startOfKthBin, size_t lengthOfKthBin, int shiftRightAmount, unsigned bitMask, size_t kthBin)
{
    size_t endOfKthBin = startOfKthBin + lengthOfKthBin;
    size_t endOfOb = startOfOb + lengthOfOb;
    size_t _current_ob = startOfOb, _current_ib = startOfKthBin; // _ob = outside of bin, _ib = inside of bin
    while (true)
    {
        // Look for the element that belongs in the bin that k is in, to move into that bin
        for (; _current_ob < endOfOb; _current_ob++)
            if (((a[_current_ob] >> shiftRightAmount) & bitMask) == kthBin) break;
        // Look for the first location in the bin that k is in, which has an element that does not belong in that bin
        if (_current_ob < endOfOb)
            for (; _current_ib < endOfKthBin; _current_ib++)
                if (((a[_current_ib] >> shiftRightAmount) & bitMask) != kthBin) break;

        if (_current_ob >= endOfOb || _current_ib >= endOfKthBin) break; // All the element outside the bin have been exhausted or the bin that k is in is full or 
        a[_current_ib++] = a[_current_ob++];    // Move the element that belongs in the bin into the bin
    }
    return _current_ib;
}
// Move elements outside the k-th bin, the bin that k is in, which belong to the k-th bin, into the k-th bin.
 // Generic implementation that work for regions to the left or to the right of the k-th bin, and for any digit size.
inline static size_t MoveOutsideOfKthBinInAndCount(unsigned a[], size_t startOfOb, size_t lengthOfOb, size_t startOfKthBin, size_t lengthOfKthBin, int shiftRightAmount, unsigned bitMask, size_t kthBin, size_t* count)
{
    size_t endOfKthBin = startOfKthBin + lengthOfKthBin;    // not inclusive
    size_t endOfOb = startOfOb + lengthOfOb;                // not inclusive
    size_t _current_ob = startOfOb;
    size_t _current_ib = startOfKthBin; // _ob = outside of bin, _ib = inside of bin
    int shiftRightAmountNextDigit = shiftRightAmount - Log2ofPowerOfTwoRadix;
    while (true)
    {
        // Look for the element that belongs in the bin that k is in, to move into that bin
        for (; _current_ob < endOfOb; _current_ob++)
            if (((a[_current_ob] >> shiftRightAmount) & bitMask) == kthBin) break;
        // Look for the first location in the bin that k is in, which has an element that does not belong in that bin
        if (_current_ob < endOfOb)
            for (; _current_ib < endOfKthBin; _current_ib++)
                if (((a[_current_ib] >> shiftRightAmount) & bitMask) != kthBin) break;
                else count[(byte)(a[_current_ib] >> shiftRightAmountNextDigit)]++;

        if (_current_ob >= endOfOb || _current_ib >= endOfKthBin) break; // All the element outside the bin have been exhausted or the bin that k is in is full or 
        count[(byte)(a[_current_ob] >> shiftRightAmountNextDigit)]++;
        a[_current_ib++] = a[_current_ob++];    // Move the element that belongs in the bin into the bin
    }
    return _current_ib;
}

inline static void RadixSelectiontNonRecursiveInner(unsigned a[], size_t first, size_t length, int shiftRightAmount, unsigned bitsPerDigit, size_t k)
{
	const unsigned BitMask = (1 << bitsPerDigit) - 1;
	const size_t NumberOfBins = 1 << bitsPerDigit;

    size_t* startOfBin = new size_t[NumberOfBins + 1];
	size_t* count      = new size_t[NumberOfBins]{};

    while (shiftRightAmount >= 0)
    {
        size_t last = first + length;  // non-inclusive
        //const auto startTime_0 = high_resolution_clock::now();
        HistogramOneComponentOpt(a, first, last, shiftRightAmount, bitsPerDigit, count);
        //const auto endTime_0 = high_resolution_clock::now();
        //printf("Histogram: Time: %fms\n", duration_cast<duration<double, milli>>(endTime_0 - startTime_0).count());
        startOfBin[0] = first; startOfBin[NumberOfBins] = last;
        for (int i = 1; i < NumberOfBins; i++)
            startOfBin[i] = startOfBin[i - 1] + count[i - 1];

        // Determine which bin contains the k-th smallest element. kthBin will hold the bin number.
        size_t kthBin = 0, _current_ib;
        for (; kthBin < NumberOfBins; kthBin++)
            if (k >= startOfBin[kthBin] && k < startOfBin[kthBin + 1]) break;

        //const auto startTime_1 = high_resolution_clock::now();
        _current_ib = MoveOutsideOfKthBinIn(a, first, startOfBin[kthBin] - first, startOfBin[kthBin], startOfBin[kthBin + 1] - startOfBin[kthBin], shiftRightAmount, BitMask, kthBin);
        //const auto endTime_1 = high_resolution_clock::now();
        //printf("Move Outside of Kth Bin #1: Time: %fms\n", duration_cast<duration<double, milli>>(endTime_1 - startTime_1).count());
        //const auto startTime_2 = high_resolution_clock::now();
        _current_ib = MoveOutsideOfKthBinIn(a, startOfBin[kthBin + 1], last - startOfBin[kthBin + 1] + 1, _current_ib, startOfBin[kthBin + 1] - _current_ib, shiftRightAmount, BitMask, kthBin);
        //const auto endTime_2 = high_resolution_clock::now();
        //printf("Move Outside of Kth Bin #2: Time: %fms\n", duration_cast<duration<double, milli>>(endTime_2 - startTime_2).count());

        if (shiftRightAmount == 0) break;
        if (shiftRightAmount >= Log2ofPowerOfTwoRadix) shiftRightAmount -= Log2ofPowerOfTwoRadix;
        else shiftRightAmount = 0;
        if ((startOfBin[kthBin + 1] - startOfBin[kthBin]) > 1)
        {
            first = startOfBin[kthBin];
            length = startOfBin[kthBin + 1] - startOfBin[kthBin];
        }
        else if ((startOfBin[kthBin + 1] - startOfBin[kthBin]) == 1) break; // Only one element in the bin that k is in, so it must be the k-th smallest element
        //TODO: Port to C++: else throw new Exception("RadixSelectiontMsdInner: No elements in the bin that k is in, which should never happen");
    }
    delete[] count;
    delete[] startOfBin;
}

inline static void RadixSelectiontNonRecursiveInner2(unsigned a[], size_t first, size_t length, int shiftRightAmount, size_t k)
{
    size_t last = first + length;
    const unsigned bitMask = PowerOfTwoRadix - 1;

    const auto startTime_0 = high_resolution_clock::now();
    size_t* count = HistogramOneByteComponent(a, first, last, shiftRightAmount);
    const auto endTime_0 = high_resolution_clock::now();
    printf("Histogram #1: Time: %fms\n", duration_cast<duration<double, milli>>(endTime_0 - startTime_0).count());
    size_t* startOfBin = new size_t[PowerOfTwoRadix + 1];

    while (shiftRightAmount >= 0)
    {
        startOfBin[0] = first; startOfBin[PowerOfTwoRadix] = last + 1;
        for (int i = 1; i < PowerOfTwoRadix; i++)
            startOfBin[i] = startOfBin[i - 1] + count[i - 1];
        for (int i = 0; i < PowerOfTwoRadix; i++)
            count[i] = 0;

        // Determine which bin contains the k-th smallest element. kthBin will hold the bin number.
        size_t kthBin = 0, _current_ib;
        for (; kthBin < PowerOfTwoRadix; kthBin++)
        {
            size_t binLength = startOfBin[kthBin + 1] - startOfBin[kthBin];
            if (binLength == 0) continue; // skip empty bins
            if (k >= startOfBin[kthBin] && k <= (startOfBin[kthBin + 1] - 1)) break;
        }
        const auto startTime_1 = high_resolution_clock::now();
        _current_ib = MoveOutsideOfKthBinInAndCount(a, first, startOfBin[kthBin] - first, startOfBin[kthBin], startOfBin[kthBin + 1] - startOfBin[kthBin], shiftRightAmount, bitMask, kthBin, count);
        const auto endTime_1 = high_resolution_clock::now();
        printf("Move Outside of Kth Bin #1: Time: %fms\n", duration_cast<duration<double, milli>>(endTime_1 - startTime_1).count());
        const auto startTime_2 = high_resolution_clock::now();
        _current_ib = MoveOutsideOfKthBinInAndCount(a, startOfBin[kthBin + 1], last - startOfBin[kthBin + 1] + 1, _current_ib, startOfBin[kthBin + 1] - _current_ib, shiftRightAmount, bitMask, kthBin, count);
        const auto endTime_2 = high_resolution_clock::now();
        printf("Move Outside of Kth Bin #2: Time: %fms\n", duration_cast<duration<double, milli>>(endTime_2 - startTime_2).count());

        int shiftRightAmountNextDigit = shiftRightAmount - Log2ofPowerOfTwoRadix;
        if (shiftRightAmount > 0)
        {
            const auto startTime_3 = high_resolution_clock::now();
            for (; _current_ib < startOfBin[kthBin + 1]; _current_ib++)
                count[(byte)(a[_current_ib] >> shiftRightAmountNextDigit)]++;

            //HistogramOneByteComponentOpt(a, _current_ib, startOfBin[kthBin + 1] - 1, shiftRightAmountNextDigit, count);

            const auto endTime_3 = high_resolution_clock::now();
            printf("Histogram #2: Time: %fms\n", duration_cast<duration<double, milli>>(endTime_3 - startTime_3).count());
        }

        if (shiftRightAmount == 0) break;
        if (shiftRightAmount >= Log2ofPowerOfTwoRadix) shiftRightAmount -= Log2ofPowerOfTwoRadix;
        else shiftRightAmount = 0;
        // Only recurse into the bin that contains the k-th smallest element and if more than one element is in that bin
        if ((startOfBin[kthBin + 1] - startOfBin[kthBin]) > 1)
        {
            first = startOfBin[kthBin];
            length = startOfBin[kthBin + 1] - startOfBin[kthBin];
            last = first + length - 1;
        }
        else if ((startOfBin[kthBin + 1] - startOfBin[kthBin]) == 1) return; // Only one element in the bin that k is in, so it must be the k-th smallest element
        //TODO: Port to C++: else throw new Exception("RadixSelectiontMsdInner: No elements in the bin that k is in, which should never happen");
    }
	delete[] startOfBin;
	delete[] count;
}

/**
 * @brief In-place Radix Selection of the k-th element in an array. Processes one byte-digits at a time.
 * @param arrayToBeSelected Array that is to be selected from in place
 * @param start Starting index of the subarray
 * @param length Length of the subarray
 * @param k Index of the desired element to be selected
 * @return the k-th element in the array
 */
inline unsigned SelectRadix(unsigned arrayToBeSelected[], size_t start, size_t length, size_t k)
{
    //if (arrayToBeSelected == null)
    //    throw new ArgumentNullException(nameof(arrayToBeSelected));
    //if (start < 0 || length <= 0)
    //    throw new ArgumentOutOfRangeException(nameof(k), "l or r are invalid");
    //if (k < start || k >(start + arrayToBeSelected.Length))
    //    throw new ArgumentOutOfRangeException(nameof(k), "k must be between start and (start + length)");
    int shiftRightAmount = (sizeof(unsigned) * 8) - Log2ofPowerOfTwoRadix;
    RadixSelectiontNonRecursiveInner(arrayToBeSelected, start, length, shiftRightAmount, Log2ofPowerOfTwoRadix, k);
    return arrayToBeSelected[k];
}
/**
 * @brief In-place Radix Selection of the k-th element in an array. Processes one byte-digits at a time.
 * @param ArrayToBeSelected array that is to be sorted in place
 * @param k Index of the desired element to be selected
 * @return the k-th element in the array
 */
inline unsigned SelectRadix(unsigned arrayToBeSelected[], size_t length, size_t k)
{
    //if (arrayToBeSelected == null)
    //    throw new ArgumentNullException(nameof(arrayToBeSelected));
    //if (arrayToBeSelected.Length <= 0)
    //    throw new ArgumentOutOfRangeException(nameof(arrayToBeSelected.Length), "array length is invalid");
    //if (k < 0 || k > arrayToBeSelected.Length)
    //    throw new ArgumentOutOfRangeException(nameof(k), "k must be between start and (start + length)");
    int shiftRightAmount = (sizeof(unsigned) * 8) - Log2ofPowerOfTwoRadix;
    RadixSelectiontNonRecursiveInner(arrayToBeSelected, 0, length, shiftRightAmount, Log2ofPowerOfTwoRadix, k);
    return arrayToBeSelected[k];
}

/**
 * @brief In-place Radix Selection of the k-th element in an array. Processes one word-digits (16-bits) at a time.
 * @param arrayToBeSelected Array that is to be selected from in place
 * @param start Starting index of the subarray
 * @param length Length of the subarray
 * @param k Index of the desired element to be selected
 * @return the k-th element in the array
 */
inline unsigned SelectRadixWord(unsigned arrayToBeSelected[], size_t start, size_t length, size_t k)
{
    //if (arrayToBeSelected == null)
    //    throw new ArgumentNullException(nameof(arrayToBeSelected));
    //if (start < 0 || length <= 0)
    //    throw new ArgumentOutOfRangeException(nameof(k), "l or r are invalid");
    //if (k < start || k >(start + arrayToBeSelected.Length))
    //    throw new ArgumentOutOfRangeException(nameof(k), "k must be between start and (start + length)");
    int shiftRightAmount = (sizeof(unsigned) * 8) - 16;
    RadixSelectiontNonRecursiveInner(arrayToBeSelected, start, length, shiftRightAmount, 16, k);
    return arrayToBeSelected[k];
}
/**
 * @brief In-place Radix Selection of the k-th element in an array. Processes one word-digits (16-bits) at a time.
 * @param ArrayToBeSelected array that is to be sorted in place
 * @param k Index of the desired element to be selected
 * @return the k-th element in the array
 */
inline unsigned SelectRadixWord(unsigned arrayToBeSelected[], size_t length, size_t k)
{
    //if (arrayToBeSelected == null)
    //    throw new ArgumentNullException(nameof(arrayToBeSelected));
    //if (arrayToBeSelected.Length <= 0)
    //    throw new ArgumentOutOfRangeException(nameof(arrayToBeSelected.Length), "array length is invalid");
    //if (k < 0 || k > arrayToBeSelected.Length)
    //    throw new ArgumentOutOfRangeException(nameof(k), "k must be between start and (start + length)");
    int shiftRightAmount = (sizeof(unsigned) * 8) - 16;
    RadixSelectiontNonRecursiveInner(arrayToBeSelected, 0, length, shiftRightAmount, 16, k);
    return arrayToBeSelected[k];
}

#endif