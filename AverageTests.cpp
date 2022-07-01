#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <ratio>
#include <vector>
#include <execution>

int AverageOverflowFree(int a, int b)
{
	int average_ab;

	if ((a >= 0 && b >= 0) || (a < 0 && b < 0))
		average_ab = a + (b - a) / 2;
	else
		average_ab = (a + b) / 2;

	return average_ab;
}

long long AverageOverflowFree(long long a, long long b)
{
	long long average_ab;

	if ((a >= 0 && b >= 0) || (a < 0 && b < 0))
		average_ab = a + (b - a) / 2;
	else
		average_ab = (a + b) / 2;

	return average_ab;
}
unsigned int AverageUnderflowFree(unsigned int a, unsigned int b)
{
	unsigned int average_ab;

	if (b >= a)
		average_ab = a + (b - a) / 2;
	else
		average_ab = b + (a - b) / 2;

	return average_ab;
}

size_t AverageUnderflowFree(size_t a, size_t b)
{
	size_t average_ab;

	if (b >= a)
		average_ab = a + (b - a) / 2;
	else
		average_ab = b + (a - b) / 2;

	return average_ab;
}

size_t AverageUnderflowFreeModulo(size_t a, size_t b)
{
	return  a / 2 + b / 2 + (a % 2 + b % 2) / 2;
}

void TestAverageOfTwoIntegers()
{
	unsigned c_u = 6;
	unsigned d_u = 8;
	unsigned ave_cd_0 = (c_u + d_u) / 2;        // correct result of 7
	unsigned ave_cd_1 = c_u + (c_u - d_u) / 2;  // wrong result of 2147483653
	unsigned ave_cd_2 = c_u + (d_u - c_u) / 2;	// correct result of 7

	printf("Average #0 = %u   Average #1 = %u   Average #2 = %u\n", ave_cd_0, ave_cd_1, ave_cd_2);

	unsigned a_u = 1;
	unsigned b_u = UINT32_MAX;

	unsigned ave_ab_0 = (a_u + b_u) / 2;        // wrong result of 0
	unsigned ave_ab_1 = a_u + (b_u - a_u) / 2;  // wrong result of 2
	unsigned ave_ab_2 = a_u + (b_u - a_u) / 2;	// correct result of 2147483648
	unsigned ave_ab_3 = b_u + (a_u - b_u) / 2;	// wrong result of 0

	printf("Average #0 = %u   Average #1 = %u   Average #2 = %u   Average #3 = %u   (a_u - b_u) = %u   AverageMod = %u\n",
		ave_ab_0, ave_ab_1, ave_ab_2, ave_ab_3, (a_u - b_u), (unsigned)AverageUnderflowFreeModulo(a_u, b_u));

	int e_i = 1;
	int f_i = INT32_MAX;

	int ave_ef_0 = (e_i + f_i) / 2;         // wrong result of -1073741824
	int ave_ef_1 = e_i + (e_i - f_i) / 2;   // wrong result of -1073741822
	int ave_ef_2 = e_i + (f_i - e_i) / 2;	// correct result of 1073741824

	printf("Average #0 = %d   Average #1 = %d   Average #2 = %d\n", ave_ef_0, ave_ef_1, ave_ef_2);

	e_i = -1;
	f_i = INT32_MIN;

	ave_ef_0 = (e_i + f_i) / 2;         // wrong result of 1073741823
	ave_ef_1 = e_i + (e_i - f_i) / 2;   // wrong result of 1073741822
	ave_ef_2 = e_i + (f_i - e_i) / 2;	// correct result of -1073741824

	printf("Average #0 = %d   Average #1 = %d   Average #2 = %d\n", ave_ef_0, ave_ef_1, ave_ef_2);

	// Idea for unsigned: compare the two values, use the case of (larger - smaller)
	// Idea for signed: compare the two values with zero, if both negative then compare to each other and use (smaller - larger)
	// if both positive then compare to each other and use (larger - smaller), if oposite signs then can subtract without comparing to each other
	// Another clever solution for integers if you know they will be positive: ave = ((unsigned)low + (unsigned)high) / 2 . This works because
	// If we know that high >= low, then int mid = low + ((high - low) / 2 ) works

	if (f_i >= e_i)
		ave_ef_2 = e_i + (f_i - e_i) / 2;
	else
		ave_ef_2 = e_i + (e_i - f_i) / 2;

	printf("\n\n");
	printf("AverageSafe = %d   input A = %d   input B = %d\n", AverageOverflowFree(       -1, INT32_MIN),        -1,  INT32_MIN);
	printf("AverageSafe = %d   input A = %d   input B = %d\n", AverageOverflowFree(INT32_MIN,        -1), INT32_MIN,         -1);
	printf("AverageSafe = %d   input A = %d   input B = %d\n", AverageOverflowFree(        1, INT32_MAX),         1,  INT32_MAX);
	printf("AverageSafe = %d   input A = %d   input B = %d\n", AverageOverflowFree(INT32_MAX,         1), INT32_MAX,          1);
	printf("AverageSafe = %d   input A = %d   input B = %d\n", AverageOverflowFree(INT32_MAX, INT32_MIN), INT32_MAX,  INT32_MIN);
	printf("AverageSafe = %d   input A = %d   input B = %d\n", AverageOverflowFree(INT32_MIN, INT32_MAX), INT32_MIN,  INT32_MAX);
	printf("AverageSafe = %d   input A = %d   input B = %d\n", AverageOverflowFree(        5,        -1),         5,         -1);
	printf("AverageSafe = %d   input A = %d   input B = %d\n", AverageOverflowFree(       -1,         5),        -1,          5);

}
