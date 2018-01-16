// ParallelAlgorithms.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include "ParallelMergeSort.h"

int main()
{
	const unsigned long NumElements = 7;
	unsigned long unsorted_array[NumElements] = { 10, 5, 2, 4, 11, 0, 3 };
	unsigned long   sorted_array[NumElements];

	parallel_merge_sort_simplest(unsorted_array, 0, NumElements - 1, sorted_array);

	for (unsigned long i = 0; i < NumElements; i++)
		std::cout << sorted_array[i] << " ";
	std::cout << std::endl;

    return 0;
}
