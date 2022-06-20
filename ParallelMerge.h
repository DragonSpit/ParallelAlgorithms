// Parallel Merge implementations
// TODO: Need to expose parallel threshold to the user
// TODO: Merge and Parallel Merge need indexes to be size_t instead of int
// TODO: Convert all Merge function to use [left, right) boundary method where left is included and right is not, to allow specification of zero length array
//       even at left being at zero index. This will go well with size_t transition.

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
#include <tbb/parallel_invoke.h>
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
inline void merge_parallel_L5(_Type* t, size_t p1, size_t r1, size_t p2, size_t r2, _Type* a, size_t p3)
{
	size_t length1 = r1 - p1 + 1;
	size_t length2 = r2 - p2 + 1;
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
		size_t q1 = (p1 + r1) / 2;
		size_t q2 = my_binary_search(t[q1], t, p2, r2);
		size_t q3 = p3 + (q1 - p1) + (q2 - p2);
		a[q3] = t[q1];
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { merge_parallel_L5(t, p1,     q1 - 1, p2, q2 - 1, a, p3    ); },
			[&] { merge_parallel_L5(t, q1 + 1, r1,     q2, r2,     a, q3 + 1); }
		);
	}
}

template< class _Type >
inline void merge_parallel_quad(_Type* t, size_t p1, size_t r1, size_t p2, size_t r2, _Type* a, size_t p3)
{
	size_t length1 = r1 - p1 + 1;
	size_t length2 = r2 - p2 + 1;
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
		size_t q1 = (p1 + r1) / 2;
		size_t q2 = my_binary_search(t[q1], t, p2, r2);
		size_t q3 = p3 + (q1 - p1) + (q2 - p2);
		a[q3] = t[q1];
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { merge_parallel_quad(t, p1,     q1 - 1, p2, q2 - 1, a, p3    ); },
			[&] { merge_parallel_quad(t, q1 + 1, r1,     q2, r2,     a, q3 + 1); }
		);
	}
}

template< class _Type >
inline void mirror_ptr(_Type* a, int l, int r)
{
	while (l < r)	exchange(a[l++], a[r--]);
}

// Swaps two sequential sub-arrays ranges a[ l .. m ] and a[ m + 1 .. r ]
// Seems to be the fastest version, as if mirror of the two blocks brings into the cache the most of the whole array for the last mirror to do.
template< class _Type >
inline void block_exchange_mirror(_Type* a, int l, int m, int r)
{
	mirror_ptr(a, l,     m);
	mirror_ptr(a, m + 1, r);
	mirror_ptr(a, l,     r);
}

// Swaps two sequential sub-arrays ranges a[ l .. m ] and a[ m + 1 .. r ]
// Seems to be the fastest version, as if mirror of the two blocks brings into the cache the most of the whole array for the last mirror to do.
template< class _Type >
inline void block_exchange_mirror_par(_Type* a, int l, int m, int r, int threshold = 64 * 1024)
{
	int length = r - l + 1;
	if (length < threshold)
		block_exchange_mirror(a, l, m, r);
	else
	{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { mirror_ptr(a, l,     m); },
			[&] { mirror_ptr(a, m + 1, r); }
		);
		mirror_ptr(a, l, r);
	}
}

// Merge two ranges of source array T[ l .. m, m+1 .. r ] in-place.
// Based on not-in-place algorithm in 3rd ed. of "Introduction to Algorithms" p. 798-802, extending it to be in-place
// and my Dr. Dobb's paper https://www.drdobbs.com/parallel/parallel-in-place-merge/240008783 or https://web.archive.org/web/20141217133856/http://www.drdobbs.com/parallel/parallel-in-place-merge/240008783
template< class _Type >
inline void p_merge_in_place_2(_Type* t, int l, int m, int r)
{
	int length1 = m - l + 1;
	int length2 = r - m;
	if (length1 >= length2)
	{
		if (length2 <= 0)	return;
		//		if ( length1 <= 32 && length2 <= 32 )	{ mergeInPlace( t, l, m, r );  return; }
		//		if (( length1 <= 16*1024 ) && ( length2 <= 16*1024 ))	{ _mergeSedgewick( t, l, m, r );  return; }
		//		if (( length1 + length2 ) <= 1024 )	{ mergeSedgewick_small_arrays_only< 1024 >( t, l, m, r );  return; }	// 2X speedup
		if ((length1 + length2) <= 1024) { std::inplace_merge(t + l, t + m + 1, t + r + 1);  return; }
		//		if ( length1 < 1024 )	{ merge_inplace_forward< 1024 >( t, l, m, r );  return; }	
		int q1 = (l + m) / 2;								// q1 is mid-point of the larger segment
		int q2 = my_binary_search(t[q1], t, m + 1, r);	// q2 is q1 partitioning element within the smaller sub-array (and q2 itself is part of the sub-array that does not move)
		int q3 = q1 + (q2 - m - 1);
		//		block_exchange_7< 16 >( t, q1, m, q2 - 1 );
		//		block_exchange_mirror_reverse_order(( t, q1, m, q2 - 1 );
		//		p_block_exchange( t, q1, m, q2 - 1 );
		//block_exchange_mirror(t, q1, m, q2 - 1);		// 2X speedup
		block_exchange_mirror_par(t, q1, m, q2 - 1);
//		block_exchange_juggling_Bentley( &t[ q1 ], q1 - q1, m - q1, q2 - 1 - q1 );
//		block_swap_Bentley( &t[ q1 ], q1 - q1, m - q1, q2 - 1 - q1 );
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { p_merge_in_place_2(t, l,      q1 - 1, q3 - 1); },	// note that q3 is now in its final place and no longer participates in further processing
			[&] { p_merge_in_place_2(t, q3 + 1, q2 - 1, r     ); }
		);
	}
	else {
		if (length1 <= 0)	return;
		//		if ( length1 <= 32 && length2 <= 32 )	{ mergeInPlace( t, l, m, r );  return; }
		//		if (( length1 <= 16*1024 ) && ( length2 <= 16*1024 ))	{ _mergeSedgewick( t, l, m, r );  return; }
		//		if (( length1 + length2 ) <= 1024 )	{ mergeSedgewick_small_arrays_only< 1024 >( t, l, m, r );  return; }	// 2X speedup
		if ((length1 + length2) <= 1024) { std::inplace_merge(t + l, t + m + 1, t + r + 1);  return; }
		//		if ( length2 < 1024 )	{ merge_inplace_reverse< 1024 >( t, l, m, r );  return; }	
		int q1 = (m + 1 + r) / 2;							// q1 is mid-point of the larger segment
		int q2 = my_binary_search(t[q1], t, l, m);		// q2 is q1 partitioning element within the smaller sub-array (and q2 itself is part of the sub-array that does not move)
		int q3 = q2 + (q1 - m - 1);
		//		block_exchange_7< 16 >( t, q2, m, q1 );
		//		block_exchange_mirror_reverse_order(( t, q2, m, q1 );
		//		p_block_exchange( t, q2, m, q1 );
		//block_exchange_mirror(t, q2, m, q1);			// 2X speedup
		block_exchange_mirror_par(t, q2, m, q1);
//		block_exchange_juggling_Bentley( &t[ q2 ], q2 - q2, m - q2, q1 - q2 );
//		block_swap_Bentley( &t[ q2 ], q2 - q2, m - q2, q1 - q2 );
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
		Concurrency::parallel_invoke(
#else
		tbb::parallel_invoke(
#endif
			[&] { p_merge_in_place_2(t, l, q2 - 1, q3 - 1); },	// note that q3 is now in its final place and no longer participates in further processing
			[&] { p_merge_in_place_2(t, q3 + 1, q1, r); }
		);
	}
}


#endif