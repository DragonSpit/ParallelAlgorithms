// Copyright(c), Victor J. Duvanenko, 2010
// Common items for Radix Sort implementations.

#ifndef _RadixSortCommon_h
#define _RadixSortCommon_h

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


#endif	// _CommonRadixSort_h