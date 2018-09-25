// Parallel Merge implementations

#ifndef _ParallelMerge_h
#define _ParallelMerge_h

#include "InsertionSort.h"
#include "BinarySearch.h"
#include <ppl.h>

template < class Item >
inline void exchange(Item& A, Item& B)
{
	Item t = A;
	A = B;
	B = t;
}

// Listing 1
// _end pointer point not to the last element, but one past and never access it.
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
	//tbb::parallel_invoke(
	Concurrency::parallel_invoke(
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
	if ((length1 + length2) <= 8192) {	// 8192 threshold is much better than 16
		merge_ptr_1( &t[ p1 ], &t[ p1 + length1 ], &t[ p2 ], &t[ p2 + length2 ], &a[ p3 ] );	// in DDJ paper
	}
	else {
		int q1 = (p1 + r1) / 2;
		int q2 = my_binary_search(t[q1], t, p2, r2);
		int q3 = p3 + (q1 - p1) + (q2 - p2);
		a[q3] = t[q1];
		//tbb::parallel_invoke(
		Concurrency::parallel_invoke(
			[&] { merge_parallel_L5(t, p1, q1 - 1, p2, q2 - 1, a, p3); },
			[&] { merge_parallel_L5(t, q1 + 1, r1, q2, r2, a, q3 + 1); }
		);
	}
}

#endif