// Parallel Merge Sort implementations

#ifndef _ParallelMergeSort_h
#define _ParallelMergeSort_h

#include "InsertionSort.h"
#include "BinarySearch.h"
#include "ParallelMerge.h"
#include <ppl.h>

// The simplest version of parallel merge sort that reverses direction of source and destination arrays on each level of recursion
// to eliminate the use of an additional array.  The top-level of recursion starts in the source to destination direction, which is
// what's needed and reverses direction at each level of recursion, handling the leaf nodes by using a copy when the direction is opposite.
// Assumes l <= r on entrance, which is simple to check if really needed.
// Think of srcDst as specifying the direction at this recursion level, and as recursion goes down what is passed in srcDst is control of
// direction for that next level of recursion.
// Will this work if the top-level srcToDst is set to false to begin with - i.e. we want the result to end up in the source buffer and use
// the destination buffer as an auxilary buffer/storage.  It would be really cool if the algorithm just worked this way, and had these
// two modes of usage.  I predict that it will just work that way, and then I may need to define two entrance point functions that make these
// two behaviors more obvious and explicit and not even have srcToDst argument.
// Indexes l and r must be int's to provide the ability to specify zero elements with l = 0 and r = -1.  Otherwise, specifying zero would be a little strange
// and you'd have to do it as l = 1 and r = 0. !!! This may be the reason that STL does *src_start and *src_end, and then the wrapper function may not be needed!!!

// Listing 1
template< class _Type >
inline void parallel_merge_sort_simplest_r( _Type* src, int l, int r, _Type* dst, bool srcToDst = true )	// srcToDst specifies direction for this level of recursion
{
	if ( r == l ) {    // termination/base case of sorting a single element
		if ( srcToDst )  dst[ l ] = src[ l ];    // copy the single element from src to dst
		return;
	}
	int m = ( r + l ) / 2;
	//tbb::parallel_invoke(				// Intel's     Threading Building Blocks (TBB)
	Concurrency::parallel_invoke(		// Microsoft's Parallel Pattern Library  (PPL)
		[&] { parallel_merge_sort_simplest_r( src, l,     m, dst, !srcToDst ); },		// reverse direction of srcToDst for the next level of recursion
		[&] { parallel_merge_sort_simplest_r( src, m + 1, r, dst, !srcToDst ); }		// reverse direction of srcToDst for the next level of recursion
	);
	if ( srcToDst ) merge_parallel_L5( src, l, m, m + 1, r, dst, l );
	else			merge_parallel_L5( dst, l, m, m + 1, r, src, l );
}

template< class _Type >
inline void parallel_merge_sort_simplest( _Type* src, int l, int r, _Type* dst, bool srcToDst = true )	// srcToDst specifies direction for this level of recursion
{
	if ( r < l ) return;
	parallel_merge_sort_simplest_r( src, l, r, dst, srcToDst );
}

// Listing 2
template< class _Type >
inline void parallel_merge_sort( _Type* src, int l, int r, _Type* dst )
{
    parallel_merge_sort_hybrid( src, l, r, dst, true );  // srcToDst = true
}
 
template< class _Type >
inline void parallel_merge_sort_pseudo_inplace( _Type* srcDst, int l, int r, _Type* aux )
{
    parallel_merge_sort_hybrid( srcDst, l, r, aux, false );  // srcToDst = false
}

// Listing 3
template< class _Type >
inline void parallel_merge_sort_hybrid_rh( _Type* src, int l, int r, _Type* dst, bool srcToDst = true )
{
    if ( r == l ) {    // termination/base case of sorting a single element
        if ( srcToDst )  dst[ l ] = src[ l ];    // copy the single element from src to dst
        return;
    }
    if (( r - l ) <= 48 ) {
        insertionSortSimilarToSTLnoSelfAssignment( src + l, r - l + 1 );        // in both cases sort the src
        //stable_sort( src + l, src + r + 1 );  // STL stable_sort can be used instead, but is slightly slower than Insertion Sort
        if ( srcToDst ) for( int i = l; i <= r; i++ )    dst[ i ] = src[ i ];    // copy from src to dst, when the result needs to be in dst
        return; 
    }
    int m = ( r + l ) / 2;
    //tbb::parallel_invoke(             // Intel's     Threading Building Blocks (TBB)
    Concurrency::parallel_invoke(       // Microsoft's Parallel Pattern Library  (PPL)
        [&] { parallel_merge_sort_hybrid_rh( src, l,     m, dst, !srcToDst ); },        // reverse direction of srcToDst for the next level of recursion
        [&] { parallel_merge_sort_hybrid_rh( src, m + 1, r, dst, !srcToDst ); }     // reverse direction of srcToDst for the next level of recursion
    );
    if ( srcToDst ) merge_parallel_L5( src, l, m, m + 1, r, dst, l );
    else            merge_parallel_L5( dst, l, m, m + 1, r, src, l );
}

// Listing 4
template< class _Type >
inline void parallel_merge_sort_hybrid_rh_1( _Type* src, int l, int r, _Type* dst, bool srcToDst = true )
{
    if ( r == l ) {    // termination/base case of sorting a single element
        if ( srcToDst )  dst[ l ] = src[ l ];    // copy the single element from src to dst
        return;
    }
    if (( r - l ) <= 48 && !srcToDst ) {     // 32 or 64 or larger seem to perform well
        insertionSortSimilarToSTLnoSelfAssignment( src + l, r - l + 1 );    // want to do dstToSrc, can just do it in-place, just sort the src, no need to copy
        //stable_sort( src + l, src + r + 1 );  // STL stable_sort can be used instead, but is slightly slower than Insertion Sort
        return; 
    }
    int m = (( r + l ) / 2 );
    //tbb::parallel_invoke(             // Intel's     Threading Building Blocks (TBB)
    Concurrency::parallel_invoke(       // Microsoft's Parallel Pattern Library  (PPL)
        [&] { parallel_merge_sort_hybrid_rh_1( src, l,     m, dst, !srcToDst ); },      // reverse direction of srcToDst for the next level of recursion
        [&] { parallel_merge_sort_hybrid_rh_1( src, m + 1, r, dst, !srcToDst ); }       // reverse direction of srcToDst for the next level of recursion
    );
    if ( srcToDst ) merge_parallel_L5( src, l, m, m + 1, r, dst, l );
    else            merge_parallel_L5( dst, l, m, m + 1, r, src, l );
}

#endif