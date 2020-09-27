// ParallelAlgorithms.cpp : Linux benchmark of standard C++ parallel algorithms versus my implementations
//

#include <stddef.h>
#include <stdio.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <ratio>
#include <vector>
#include <execution>
#include <thread>

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;
using std::random_device;
using std::sort;
using std::vector;

const int iterationCount = 5;

void print_results(const char *const tag, const vector<double>& sorted,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime)
{
	printf("%s: Lowest: %g Highest: %g Time: %fms\n", tag, sorted.front(), sorted.back(),
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

void print_results(const char* const tag, const vector<unsigned long>& sorted,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime)
{
	printf("%s: Lowest: %lu Highest: %lu Time: %fms\n", tag, sorted.front(), sorted.back(),
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

void print_results(const char* const tag, const unsigned long* sorted, size_t sortedLength,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime) {
	printf("%s: Lowest: %lu Highest: %lu Time: %fms\n", tag,
		sorted[0], sorted[sortedLength - 1],
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}


void print_results(const char* const tag, const vector<unsigned>& sorted,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime)
{
	printf("%s: Lowest: %u Highest: %u Time: %fms\n", tag, sorted.front(), sorted.back(),
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

void print_results(const char *const tag, double first, double last,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime)
{
	printf("%s: Lowest: %g Highest: %g Time: %fms\n", tag, first, last,
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

int ParallelStdCppExample(vector<double>& doubles)
{
	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		vector<double> sorted(doubles);
		const auto startTime = high_resolution_clock::now();
		sort(sorted.begin(), sorted.end());
		const auto endTime = high_resolution_clock::now();
		print_results("Serial", sorted, startTime, endTime);
	}

	for (int i = 0; i < iterationCount; ++i)
	{
		vector<double> sorted(doubles);
		const auto startTime = high_resolution_clock::now();
		// same sort call as above, but with par_unseq:
		sort(std::execution::par_unseq, sorted.begin(), sorted.end());
		const auto endTime = high_resolution_clock::now();
		// in our output, note that these are the parallel results:
		print_results("Parallel", sorted, startTime, endTime);
	}

	for (int i = 0; i < iterationCount; ++i)
	{
		double * s = new double[doubles.size()];
		for (unsigned int j = 0; j < doubles.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			s[j] = doubles[j];
		}
		const auto startTime = high_resolution_clock::now();
		sort(std::execution::par_unseq, s, s+doubles.size());
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Array", s[0], s[doubles.size() - 1], startTime, endTime);
		delete[] s;
	}

	return 0;
}

int ParallelStdCppExample(vector<unsigned long>& ulongs)
{
	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		vector<unsigned long> sorted(ulongs);
		const auto startTime = high_resolution_clock::now();
		sort(sorted.begin(), sorted.end());
		const auto endTime = high_resolution_clock::now();
		print_results("Serial", sorted, startTime, endTime);
	}

	for (int i = 0; i < iterationCount; ++i)
	{
		vector<unsigned long> sorted(ulongs);
		const auto startTime = high_resolution_clock::now();
		// same sort call as above, but with par_unseq:
		sort(std::execution::par_unseq, sorted.begin(), sorted.end());
		const auto endTime = high_resolution_clock::now();
		// in our output, note that these are the parallel results:
		print_results("Parallel", sorted, startTime, endTime);
	}

	for (int i = 0; i < iterationCount; ++i)
	{
		unsigned long* s = new unsigned long[ulongs.size()];
		for (unsigned int j = 0; j < ulongs.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			s[j] = ulongs[j];
		}
		const auto startTime = high_resolution_clock::now();
		sort(std::execution::par_unseq, s, s + ulongs.size());
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Array", s[0], s[ulongs.size() - 1], startTime, endTime);
		delete[] s;
	}

	return 0;
}

int ParallelStdCppExample(vector<unsigned>& uints)
{
	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		vector<unsigned> sorted(uints);
		const auto startTime = high_resolution_clock::now();
		sort(sorted.begin(), sorted.end());
		const auto endTime = high_resolution_clock::now();
		print_results("Serial", sorted, startTime, endTime);
	}

	for (int i = 0; i < iterationCount; ++i)
	{
		vector<unsigned> sorted(uints);
		const auto startTime = high_resolution_clock::now();
		// same sort call as above, but with par_unseq:
		sort(std::execution::par_unseq, sorted.begin(), sorted.end());
		const auto endTime = high_resolution_clock::now();
		// in our output, note that these are the parallel results:
		print_results("Parallel", sorted, startTime, endTime);
	}

	for (int i = 0; i < iterationCount; ++i)
	{
		unsigned *s = new unsigned[uints.size()];
		for (unsigned int j = 0; j < uints.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			s[j] = uints[j];
		}
		const auto startTime = high_resolution_clock::now();
		sort(std::execution::par_unseq, s, s + uints.size());
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Array", s[0], s[uints.size() - 1], startTime, endTime);
		delete[] s;
	}

	return 0;
}

// A set of logical right shift functions to work-around the C++ issue of performing an arithmetic right shift
// for >>= operation on signed types.
inline char logicalRightShift( char a, unsigned long shiftAmount )
{
	return (char)(((unsigned char)a ) >> shiftAmount );
}
inline unsigned char logicalRightShift_ru( char a, unsigned long shiftAmount )
{
	return (((unsigned char)a ) >> shiftAmount );
}
inline short logicalRightShift( short a, unsigned long shiftAmount )
{
	return (short)(((unsigned short)a ) >> shiftAmount );
}
inline unsigned short logicalRightShift_ru( short a, unsigned long shiftAmount )
{
	return (((unsigned short)a ) >> shiftAmount );
}
inline long logicalRightShift( long a, unsigned long shiftAmount )
{
	return (long)(((unsigned long)a ) >> shiftAmount );
}
inline int logicalRightShift( int a, unsigned long shiftAmount )
{
	return (int)(((unsigned long)a ) >> shiftAmount );
}
inline unsigned long logicalRightShift_ru( long a, unsigned long shiftAmount )
{
	return (((unsigned long)a ) >> shiftAmount );
}
inline unsigned long logicalRightShift_ru( int a, unsigned long shiftAmount )
{
	return (((unsigned long)a ) >> shiftAmount );
}
#if 0
inline __int64 logicalRightShift( __int64 a, unsigned long shiftAmount )
{
	return (__int64)(((unsigned __int64)a ) >> shiftAmount );
}
inline unsigned __int64 logicalRightShift_ru( __int64 a, unsigned long shiftAmount )
{
	return (((unsigned __int64)a ) >> shiftAmount );
}
#endif
template< class _Type >
inline unsigned long extractDigit( _Type a, _Type bitMask, unsigned long shiftRightAmount )
{
	unsigned long digit = (unsigned long)(( a & bitMask ) >> shiftRightAmount );	// extract the digit we are sorting based on
	return digit;
}
template< unsigned long PowerOfTwoRadix, class _Type >
inline unsigned long extractDigitNegate( _Type a, _Type bitMask, unsigned long shiftRightAmount )
{
	unsigned long digit = (unsigned long)logicalRightShift_ru((_Type)( a & bitMask ), shiftRightAmount );	// extract the digit we are sorting based on
	digit ^= ( PowerOfTwoRadix >> 1 );
	return digit;
}
// Shifts either left or right based on the sign of the shiftAmount argument.  Positive values shift left by that many bits,
// zero does not shift at all, and negative values shift right by that many bits.
template< class _Type >
inline _Type shift_left_or_right( _Type a, long shiftAmount )
{
    if ( shiftAmount >= 0 ) return a << shiftAmount;
    else                    return a >> ( -shiftAmount );
}

template< class _Type >
inline void insertionSortSimilarToSTLnoSelfAssignment( _Type* a, unsigned long a_size )
{
	for ( unsigned long i = 1; i < a_size; i++ )
	{
		if ( a[ i ] < a[ i - 1 ] )		// no need to do (j > 0) compare for the first iteration
		{
			_Type currentElement = a[ i ];
			a[ i ] = a[ i - 1 ];
			unsigned long j;
			for ( j = i - 1; j > 0 && currentElement < a[ j - 1 ]; j-- )
			{
				a[ j ] = a[ j - 1 ];
			}
			a[ j ] = currentElement;	// always necessary work/write
		}
		// Perform no work at all if the first comparison fails - i.e. never assign an element to itself!
	}
}

template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix >
inline unsigned long** HistogramByteComponents(unsigned long inArray[], int l, int r)
{
	const unsigned long numberOfDigits = Log2ofPowerOfTwoRadix;
	const unsigned long numberOfBins = PowerOfTwoRadix;

	unsigned long** count = new unsigned long* [numberOfDigits];

	for (int i = 0; i < numberOfDigits; i++)
	{
		count[i] = new unsigned long[numberOfBins];
		for (int j = 0; j < numberOfBins; j++)
			count[i][j] = 0;
	}

	for (int current = l; current <= r; current++)    // Scan the array and count the number of times each digit value appears - i.e. size of each bin
	{
		unsigned long value = inArray[current];
		count[0][value & 0xff]++;
		count[1][(value & 0xff00) >> 8]++;
		count[2][(value & 0xff0000) >> 16]++;
		count[3][(value & 0xff000000) >> 24]++;
	}
	return count;
}

// Serial LSD Radix Sort, with Counting separated into its own phase, followed by a permutation phase, as is done in HPCsharp in C#
template< unsigned long PowerOfTwoRadix, unsigned long Log2ofPowerOfTwoRadix, long Threshold>
inline void _RadixSortLSD_StableUnsigned_PowerOf2RadixScalar_TwoPhase(unsigned long* input_array, unsigned long* output_array, long last, unsigned long bitMask, unsigned long shiftRightAmount, bool inputArrayIsDestination)
{
	const unsigned long numberOfBins = PowerOfTwoRadix;
	unsigned long* _input_array = input_array;
	unsigned long* _output_array = output_array;
	bool _output_array_has_result = false;
	unsigned long currentDigit = 0;

	//const auto startTime = high_resolution_clock::now();
	unsigned long** count2D = HistogramByteComponents <PowerOfTwoRadix, Log2ofPowerOfTwoRadix>(input_array, 0, last);
	//const auto endTime = high_resolution_clock::now();
	//printf("Time for Histogram: %fms\n", duration_cast<duration<double, milli>>(endTime - startTime).count());

	alignas(64) long startOfBin[numberOfBins];
	alignas(64) long endOfBin[numberOfBins];
	//printf("endOfBin address = %p\n", endOfBin);

	while (bitMask != 0)						// end processing digits when all the mask bits have been processes and shift out, leaving none
	{
		unsigned long* count = count2D[currentDigit];

		startOfBin[0] = endOfBin[0] = 0;
		for (unsigned long i = 1; i < numberOfBins; i++)
			startOfBin[i] = endOfBin[i] = startOfBin[i - 1] + count[i - 1];

		for (long _current = 0; _current <= last; _current++)	// permutation phase
			_output_array[endOfBin[(_input_array[_current] & bitMask) >> shiftRightAmount]++] = _input_array[_current];

		bitMask <<= Log2ofPowerOfTwoRadix;
		shiftRightAmount += Log2ofPowerOfTwoRadix;
		_output_array_has_result = !_output_array_has_result;
		std::swap(_input_array, _output_array);
		currentDigit++;
	}
	// Done with processing, copy all of the bins
	if (_output_array_has_result && inputArrayIsDestination)
		for (long _current = 0; _current <= last; _current++)	// copy from output array into the input array
			_input_array[_current] = _output_array[_current];
	if (!_output_array_has_result && !inputArrayIsDestination)
		for (long _current = 0; _current <= last; _current++)	// copy from input array back into the output array
			_output_array[_current] = _input_array[_current];

	const unsigned long numberOfDigits = Log2ofPowerOfTwoRadix;	// deallocate 2D count array, which was allocated in Histogram
	for (int i = 0; i < numberOfDigits; i++)
		delete[] count2D[i];
	delete[] count2D;
}

// LSD Radix Sort - stable (LSD has to be, and this may preclude LSD Radix from being able to be in-place)
inline void RadixSortLSDPowerOf2RadixScalar_unsigned_TwoPhase(unsigned long* a, unsigned long* b, unsigned long a_size)
{
	const unsigned long Threshold = 100;	// Threshold of when to switch to using Insertion Sort
	const unsigned long PowerOfTwoRadix = 256;
	const unsigned long Log2ofPowerOfTwoRadix = 8;
	// Create bit-mask and shift right amount
	unsigned long shiftRightAmount = 0;
	unsigned long bitMask = (unsigned long)(((unsigned long)(PowerOfTwoRadix - 1)) << shiftRightAmount);	// bitMask controls/selects how many and which bits we process at a time

	// The beauty of using template arguments instead of function parameters for the Threshold and Log2ofPowerOfTwoRadix is
	// they are not pushed on the stack and are treated as constants, but local.
	if (a_size >= Threshold) {
		_RadixSortLSD_StableUnsigned_PowerOf2RadixScalar_TwoPhase< PowerOfTwoRadix, Log2ofPowerOfTwoRadix, Threshold >(a, b, a_size - 1, bitMask, shiftRightAmount, false);
	}
	else {
		// TODO: Substitute Merge Sort, as it will get rid off the for loop, since it's internal to MergeSort
		insertionSortSimilarToSTLnoSelfAssignment(a, a_size);
		for (unsigned long j = 0; j < a_size; j++)	// copy from input array to the destination array
			b[j] = a[j];
	}
}

int RadixSortLsdBenchmark(vector<unsigned long>& ulongs)
{
	random_device rd;

	// generate some random ulongs:
	unsigned long * ulongsCopy  = new unsigned long [ulongs.size()];
	//unsigned long* ulongsCopy = (unsigned long*) operator new[](sizeof(unsigned long) * ulongs.size(), (std::align_val_t)(128));
	unsigned long * sorted = new unsigned long [ulongs.size()];
	//unsigned long* sorted     = (unsigned long*) operator new[](sizeof(unsigned long) * ulongs.size(), (std::align_val_t)(128));

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		for (unsigned int j = 0; j < ulongs.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			ulongsCopy[j] = ulongs[j];
			sorted[j] = j;									// page in the destination array into system memory
		}
		printf("ulongsCopy address = %p   sorted address = %p   value at a random location = %lu %lu\n", ulongsCopy, sorted, sorted[static_cast<unsigned>(rd()) % ulongs.size()], ulongsCopy[static_cast<unsigned>(rd()) % ulongs.size()]);
		const auto startTime = high_resolution_clock::now();
		RadixSortLSDPowerOf2RadixScalar_unsigned_TwoPhase(ulongsCopy, sorted, (unsigned long)ulongs.size());
		const auto endTime = high_resolution_clock::now();
		print_results("Radix Sort LSD", sorted, ulongs.size(), startTime, endTime);
	}

	//delete[] sorted;
	//delete[](operator new[](sizeof(intptr_t) * ulongs.size(), (std::align_val_t)(64)));
	//delete [] ulongsCopy;
	//delete[](operator new[](sizeof(intptr_t) * ulongs.size(), (std::align_val_t)(64)));

	return 0;
}

template< class _Type >
inline int my_binary_search( _Type value, const _Type* a, int left, int right )
{
	long low  = left;
	long high = std::max( left, right + 1 );
	while( low < high )
	{
		long mid = ( low + high ) / 2;
		if ( value <= a[ mid ] )	high = mid;
		else						low  = mid + 1;	// because we compared to a[mid] and the value was larger than a[mid].
													// Thus, the next array element to the right from mid is the next possible
													// candidate for low, and a[mid] can not possibly be that candidate.
	}
	return high;
}

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
		tbb::parallel_invoke(
		//Concurrency::parallel_invoke(
			[&] { merge_parallel_L5(t, p1, q1 - 1, p2, q2 - 1, a, p3); },
			[&] { merge_parallel_L5(t, q1 + 1, r1, q2, r2, a, q3 + 1); }
		);
	}
}

template< class _Type >
inline void parallel_merge_sort_hybrid_rh_2(_Type* src, int l, int r, _Type* dst, bool srcToDst = true, int parallelThreshold = 24 * 1024)
{
    if (r == l) {    // termination/base case of sorting a single element
        if (srcToDst)  dst[l] = src[l];    // copy the single element from src to dst
        return;
    }
    if ((r - l) <= parallelThreshold && !srcToDst) {
        sort( src + l, src + r + 1 );
        //if (srcToDst)
        //    for (int i = l; i <= r; i++)    dst[i] = src[i];
        return;
    }
    int m = ((r + l) / 2);
    tbb::parallel_invoke(             // Intel's     Threading Building Blocks (TBB)
    //Concurrency::parallel_invoke(       // Microsoft's Parallel Pattern Library  (PPL)
        [&] { parallel_merge_sort_hybrid_rh_2(src, l, m, dst, !srcToDst); },      // reverse direction of srcToDst for the next level of recursion
        [&] { parallel_merge_sort_hybrid_rh_2(src, m + 1, r, dst, !srcToDst); }       // reverse direction of srcToDst for the next level of recursion
    );
    if (srcToDst) merge_parallel_L5(src, l, m, m + 1, r, dst, l);
    else          merge_parallel_L5(dst, l, m, m + 1, r, src, l);
}

template< class _Type >
inline void parallel_merge_sort_hybrid(_Type* src, int l, int r, _Type* dst, bool srcToDst = true, int parallelThreshold = 24 * 1024)
{
    // may return 0 when not able to detect
	
    const auto processor_count = std::thread::hardware_concurrency();
    //printf("Number of cores = %u \n", processor_count);

    if ((int)(parallelThreshold * processor_count) < (r - l + 1))
        parallelThreshold = (r - l + 1) / processor_count;

    parallel_merge_sort_hybrid_rh_2(src, l, r, dst, srcToDst, parallelThreshold);
}

void print_results(const char* const tag, const unsigned* sorted, size_t sortedLength,
	high_resolution_clock::time_point startTime,
	high_resolution_clock::time_point endTime) {
	printf("%s: Lowest: %u Highest: %u Time: %fms\n", tag,
		sorted[0], sorted[sortedLength - 1],
		duration_cast<duration<double, milli>>(endTime - startTime).count());
}

int ParallelMergeSortBenchmark(vector<unsigned>& uints)
{
	random_device rd;

	// generate some random uints:
	printf("\nBenchmarking Parallel Merge Sort Hybrid with %zu unsigned integers...\n", uints.size());
	unsigned* uintsCopy = new unsigned[uints.size()];

	// time how long it takes to sort them:
	for (int i = 0; i < iterationCount; ++i)
	{
		unsigned* sorted = new unsigned[uints.size()];
		for (unsigned int j = 0; j < uints.size(); j++) {	// copy the original random array into the source array each time, since ParallelMergeSort modifies the source array while sorting
			uintsCopy[j] = uints[j];
			sorted[j] = 0;									// page the destination array into system memory
		}
		const auto startTime = high_resolution_clock::now();
		//parallel_merge_sort_hybrid_rh_1(uintsCopy, 0, (int)(uints.size() - 1), sorted);	// ParallelMergeSort modifies the source array
		parallel_merge_sort_hybrid(uintsCopy, 0, (int)(uints.size() - 1), sorted);	// ParallelMergeSort modifies the source array
		const auto endTime = high_resolution_clock::now();
		print_results("Parallel Merge Sort", sorted, uints.size(), startTime, endTime);
		delete[] sorted;
	}

	delete[] uintsCopy;

	return 0;
}

int main()
{
	// Provide the same input random array of doubles to all sorting algorithms
	const size_t testSize = 10'000'000;
	random_device rd;
	const auto processor_count = std::thread::hardware_concurrency();
	printf("Number of cores = %u \n", processor_count);


	// generate some random doubles:
	printf("Testing with %zu random doubles...\n", testSize);
	vector<double> doubles(testSize);
	for (auto& d : doubles) {
		d = static_cast<double>(rd());
	}
	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(doubles);

	// generate some random unsigned longs:
	printf("\nTesting with %zu unsigned random longs...\n", testSize);
	vector<unsigned long> ulongs(testSize);
	for (auto& d : ulongs) {
		d = static_cast<unsigned long>(rd());
	}
	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(ulongs);

	// Benchmark the above Parallel Merge Sort algorithm
	RadixSortLsdBenchmark(ulongs);

	// generate mosly presorted unsigned long integers with a low percentage of randoms:
	printf("\nTesting with %zu nearly presorted unsigned long integers...\n", testSize);
	//vector<unsigned> uints(testSize);
	for (size_t i = 0; i < ulongs.size(); i++) {
		if ((i % 100) == 0)
			ulongs[i] = static_cast<unsigned long>(rd());
		else
			ulongs[i] = static_cast<unsigned long>(i);
	}
	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(ulongs);

	// Benchmark the above Parallel Merge Sort algorithm
	//ParallelMergeSortBenchmark(ulongs);

	// generate some random unsigned integers:
	printf("\nTesting with %zu random unsigned integers...\n", testSize);
	vector<unsigned> uints(testSize);
	for (auto& d : uints) {
		d = static_cast<unsigned>(rd());
	}
	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(uints);

	// Benchmark the above Parallel Merge Sort algorithm
	ParallelMergeSortBenchmark(uints);

	// generate mosly presorted unsigned integers with a low percentage of randoms:
	printf("\nTesting with %zu nearly presorted unsigned integers...\n", testSize);
	//vector<unsigned> uints(testSize);
	for (size_t i = 0; i < uints.size(); i++) {
		if ((i % 100000) == 0)
			uints[i] = static_cast<unsigned>(rd());
		else
			uints[i] = static_cast<unsigned>(i);
	}
	// Example of C++17 Standard C++ Parallel Sorting
	ParallelStdCppExample(uints);

	// Benchmark the above Parallel Merge Sort algorithm
	ParallelMergeSortBenchmark(uints);

	return 0;
}
