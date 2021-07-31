// Insertion Sort implementations

#ifndef _InsertionSort_h
#define _InsertionSort_h

template< class _Type >
inline void insertionSortSimilarToSTLnoSelfAssignment( _Type* a, size_t a_size )
{
	for ( size_t i = 1; i < a_size; i++ )
	{
		if ( a[ i ] < a[ i - 1 ] )		// no need to do (j > 0) compare for the first iteration
		{
			_Type currentElement = a[ i ];
			a[ i ] = a[ i - 1 ];
			size_t j;
			for ( j = i - 1; j > 0 && currentElement < a[ j - 1 ]; j-- )
			{
				a[ j ] = a[ j - 1 ];
			}
			a[ j ] = currentElement;	// always necessary work/write
		}
		// Perform no work at all if the first comparison fails - i.e. never assign an element to itself!
	}
}

#endif
