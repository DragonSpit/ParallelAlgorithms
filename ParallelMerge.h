// Parallel Merge implementations
// TODO: Need to expose parallel threshold to the user

#ifndef _ParallelMerge_h
#define _ParallelMerge_h

#include "InsertionSort.h"
#include "BinarySearch.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
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
#endif

template < class Item >
inline void exchange(Item& A, Item& B)
{
	Item t = A;
	A = B;
	B = t;
}

// Listing 1 from Dr. Dobb's Journal paper
// _end pointer point not to the last element, but one past and never access it - i.e. _end is not included
template< class _Type >
inline void merge_ptr(const _Type* a_start, const _Type* a_end, const _Type* b_start, const _Type* b_end, _Type* dst)
{
	while (a_start < a_end && b_start < b_end) {
		if (*a_start <= *b_start)	*dst++ = *a_start++;	// if elements are equal, then a[] element is output
		else						*dst++ = *b_start++;
	}
	while (a_start < a_end)	*dst++ = *a_start++;
	while (b_start < b_end)	*dst++ = *b_start++;
}
// Faster Merge: see https://duvanenko.tech.blog/2018/07/25/faster-serial-merge-in-c-and-c/
// _end pointer point not to the last element, but one past and never access it - i.e. _end is not included
template< class _Type >
inline void merge_ptr_1(const _Type* a_start, const _Type* a_end, const _Type* b_start, const _Type* b_end, _Type* dst)
{
	if (a_start < a_end && b_start < b_end) {
		while (true) {
			if (*a_start <= *b_start) {
				*dst++ = *a_start++;
				if (a_start >= a_end)	break;
			}
			else {
				*dst++ = *b_start++;
				if (b_start >= b_end)	break;
			}
		}
	}
	while (a_start < a_end)	*dst++ = *a_start++;
	while (b_start < b_end)	*dst++ = *b_start++;
}
template< class _Type >
inline void merge_ptr_1_unrolled(const _Type* a_start, const _Type* a_end, const _Type* b_start, const _Type* b_end, _Type* dst)
{
	if (a_start < a_end && b_start < b_end) {
		while (true) {
			if (*a_start <= *b_start) {
				*dst++ = *a_start++;
				if (a_start >= a_end)	break;
			}
			else {
				*dst++ = *b_start++;
				if (b_start >= b_end)	break;
			}
			if (*a_start <= *b_start) {
				*dst++ = *a_start++;
				if (a_start >= a_end)	break;
			}
			else {
				*dst++ = *b_start++;
				if (b_start >= b_end)	break;
			}
			if (*a_start <= *b_start) {
				*dst++ = *a_start++;
				if (a_start >= a_end)	break;
			}
			else {
				*dst++ = *b_start++;
				if (b_start >= b_end)	break;
			}
			if (*a_start <= *b_start) {
				*dst++ = *a_start++;
				if (a_start >= a_end)	break;
			}
			else {
				*dst++ = *b_start++;
				if (b_start >= b_end)	break;
			}
		}
	}
	while (a_start < a_end)	*dst++ = *a_start++;
	while (b_start < b_end)	*dst++ = *b_start++;
}
template< class _Type >
inline void merge_ptr_2(const _Type* a_start, const _Type* a_end, const _Type* b_start, const _Type* b_end, _Type* dst)
{
	long aLength = (long)(a_end - a_start);
	long bLength = (long)(b_end - b_start);
	while (aLength > 0 && bLength > 0)
	{
		long numElements = std::min(aLength, bLength);
		for (long i = 0; i < numElements; i++)
		{
			if (*a_start <= *b_start)   			// if elements are equal, then a[] element is output
				*dst++ = *a_start++;
			else
				*dst++ = *b_start++;
		}
		aLength = (long)(a_end - a_start);
		bLength = (long)(b_end - b_start);
	}
	while (a_start < a_end)	*dst++ = *a_start++;
	while (b_start < b_end)	*dst++ = *b_start++;
}

// New merge concept, which uses a single comparison that should be easier for branch prediction
// _end pointer point not to the last element, but one past and never access it - i.e. _end is not included
template< class _Type >
inline void merge_ptr_3(const _Type* a_start, const _Type* a_end, const _Type* b_start, const _Type* b_end, _Type* dst, long threshold = 10 * 1024)
{
	while (true)
	{
		long aLength = (long)(a_end - a_start);
		long bLength = (long)(b_end - b_start);
		long numElements;

		if (aLength <= bLength)
		{
			if (aLength < threshold)
			{
				merge_ptr_2(a_start, a_end, b_start, b_end, dst);
				return;
			}
			else
				numElements = aLength;
		}
		else
		{
			if (bLength < threshold)
			{
				merge_ptr_2(a_start, a_end, b_start, b_end, dst);
				return;
			}
			else
				numElements = bLength;
		}

		_Type* dst_end = dst + numElements - 1;

		while (dst <= dst_end)                // single comparison, which should be simpler for branch prediction to predict
		{
			if (*a_start <= *b_start)         // if elements are equal, then a[] element is output
				*dst++ = *a_start++;
			else
				*dst++ = *b_start++;
		}
	}
}

template< class _Type >
inline void merge_ptr_adaptive_2(const _Type* a_start, const _Type* a_end, const _Type* b_start, const _Type* b_end, _Type* dst)
{
	long aLength = (long)(a_end - a_start);
	long bLength = (long)(b_end - b_start);
	while (aLength > 0 && bLength > 0)
	{
		long numElements = std::min(aLength, bLength);
		if (numElements < 128)
			merge_ptr_1(a_start, a_end, b_start, b_end, dst);
		for (long i = 0; i < numElements; i++)
		{
			if (*a_start <= *b_start)   			// if elements are equal, then a[] element is output
				*dst++ = *a_start++;
			else
				*dst++ = *b_start++;
		}
		aLength = (long)(a_end - a_start);
		bLength = (long)(b_end - b_start);
	}
	while (a_start < a_end)	*dst++ = *a_start++;
	while (b_start < b_end)	*dst++ = *b_start++;
}

// Listing 2 
// Divide-and-Conquer Merge of two ranges of source array T[ p1 .. r1 ] and T[ p2 .. r2 ] into destination array A starting at index p3.
// From 3rd ed. of "Introduction to Algorithms" p. 798-802
// Listing 2 (which also needs to include the binary search implementation as well)
template< class _Type >
inline void merge_dac(const _Type* t, int p1, int r1, int p2, int r2, _Type* a, int p3)
{
	int length1 = r1 - p1 + 1;
	int length2 = r2 - p2 + 1;
	if (length1 < length2)
	{
		exchange(p1, p2);
		exchange(r1, r2);
		exchange(length1, length2);
	}
	if (length1 == 0) return;
	int q1 = (p1 + r1) / 2;
	int q2 = my_binary_search(t[q1], t, p2, r2);
	int q3 = p3 + (q1 - p1) + (q2 - p2);
	a[q3] = t[q1];
	merge_dac(t, p1, q1 - 1, p2, q2 - 1, a, p3);
	merge_dac(t, q1 + 1, r1, q2, r2, a, q3 + 1);
}

// Listing 3
template< class _Type >
inline void merge_parallel_L3(_Type* t, int p1, int r1, int p2, int r2, _Type* a, int p3)
{
	int length1 = r1 - p1 + 1;
	int length2 = r2 - p2 + 1;
	if (length1 < length2) {
		exchange(p1, p2);
		exchange(r1, r2);
		exchange(length1, length2);
	}
	if (length1 == 0) return;
	int q1 = (p1 + r1) / 2;
	int q2 = my_binary_search(t[q1], t, p2, r2);
	int q3 = p3 + (q1 - p1) + (q2 - p2);
	a[q3] = t[q1];
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	Concurrency::parallel_invoke(
#else
	tbb::parallel_invoke(
#endif
		[&] { merge_parallel_L3(t, p1, q1 - 1, p2, q2 - 1, a, p3); },
		[&] { merge_parallel_L3(t, q1 + 1, r1, q2, r2, a, q3 + 1); }
	);
}

// Listing 4
// The hybrid divide-and-conquer algorithm implementation
template< class _Type >
inline void merge_dac_hybrid(const _Type* t, int p1, int r1, int p2, int r2, _Type* a, int p3)
{
	int length1 = r1 - p1 + 1;
	int length2 = r2 - p2 + 1;
	if (length1 < length2)
	{
		exchange(p1, p2);
		exchange(r1, r2);
		exchange(length1, length2);
	}
	if (length1 == 0) return;
	if ((length1 + length2) <= 8192)
		merge_ptr_1(&t[p1], &t[p1 + length1], &t[p2], &t[p2 + length2], &a[p3]);
	else {
		int q1 = (p1 + r1) / 2;
		int q2 = my_binary_search(t[q1], t, p2, r2);
		int q3 = p3 + (q1 - p1) + (q2 - p2);
		a[q3] = t[q1];
		merge_dac_hybrid(t, p1, q1 - 1, p2, q2 - 1, a, p3);
		merge_dac_hybrid(t, q1 + 1, r1, q2, r2, a, q3 + 1);
	}
}

// Listing 5
template< class _Type >
inline void merge_parallel_L5(_Type* t, int p1, int r1, int p2, int r2, _Type* a, int p3)
{
	int length1 = r1 - p1 + 1;
	int length2 = r2 - p2 + 1;
	if (length1 < length2) {
		exchange(p1, p2);
		exchange(r1, r2);
		exchange(length1, length2);
	}
	if (length1 == 0)	return;
	if ((length1 + length2) <= 32768) {	// 8192 threshold is much better than 16. 32K seems to be an even better threshold
		//merge_ptr( &t[ p1 ], &t[ p1 + length1 ], &t[ p2 ], &t[ p2 + length2 ], &a[ p3 ] );	// in DDJ paper
		merge_ptr_1(&t[p1], &t[p1 + length1], &t[p2], &t[p2 + length2], &a[p3]);				// slightly faster than merge_ptr version due to fewer loop comparisons
		//merge_ptr_3(&t[p1], &t[p1 + length1], &t[p2], &t[p2 + length2], &a[p3]);				// new merge concept, which turned out slower
	}
	else {
		int q1 = (p1 + r1) / 2;
		int q2 = my_binary_search(t[q1], t, p2, r2);
		int q3 = p3 + (q1 - p1) + (q2 - p2);
		a[q3] = t[q1];
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { merge_parallel_L5(t, p1, q1 - 1, p2, q2 - 1, a, p3); },
			[&] { merge_parallel_L5(t, q1 + 1, r1, q2, r2, a, q3 + 1); }
		);
	}
}

#endif