#pragma once

#include <algorithm>
#include "RadixSortMSD.h"
#include "RadixSortLSD.h"

inline __int64 DeDuplicate(unsigned arrayToDeDuplicate[], size_t arraySize)
{
	hybrid_inplace_msd_radix_sort_bins(arrayToDeDuplicate, (unsigned long)arraySize);			// In-place MSD Radix Sort
	auto last = std::unique(arrayToDeDuplicate, arrayToDeDuplicate + arraySize);
	auto uniqueCount = std::distance(arrayToDeDuplicate, last);
	return uniqueCount;
}

inline __int64 DeDuplicateNotInPlace(unsigned arrayToDeDuplicate[], unsigned tmp_working[], size_t arraySize)
{
	RadixSortLSDPowerOf2Radix_unsigned_TwoPhase(arrayToDeDuplicate, tmp_working, arraySize);	// Not-in-place LSD Radix Sort
	auto last = std::unique(arrayToDeDuplicate, arrayToDeDuplicate + arraySize);
	auto uniqueCount = std::distance(arrayToDeDuplicate, last);
	return uniqueCount;
}
