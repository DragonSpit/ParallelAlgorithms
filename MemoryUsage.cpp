// from https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process

#include <stddef.h>
#include <stdio.h>
#include <chrono>
#include <execution>

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include "windows.h"
#else
#include "sys/types.h"
#include "sys/sysinfo.h"
#endif

unsigned long long physical_memory_used_in_megabytes()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	//DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;
	//DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
	//DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
	DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;	// physical memory used by the system
	//SIZE_T physMemUsedByMe = pmc.WorkingSetSize;	// by current process
	return (physMemUsed / (1024ULL * 1024));
#else
	struct sysinfo memInfo;

	sysinfo(&memInfo);
	long long totalVirtualMem = memInfo.totalram;
	//Add other values in next statement to avoid int overflow on right hand side...
	totalVirtualMem += memInfo.totalswap;
	totalVirtualMem *= memInfo.mem_unit;
	long long virtualMemUsed = memInfo.totalram - memInfo.freeram;
	//Add other values in next statement to avoid int overflow on right hand side...
	virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
	virtualMemUsed *= memInfo.mem_unit;
	long long totalPhysMem = memInfo.totalram;
	//Multiply in next statement to avoid int overflow on right hand side...
	totalPhysMem *= memInfo.mem_unit;
	long long physMemUsed = memInfo.totalram - memInfo.freeram;
	//Multiply in next statement to avoid int overflow on right hand side...
	physMemUsed *= memInfo.mem_unit;	// total physical memory used by the whole system
	return (physMemUsed / (1024ULL * 1024));
#endif
}

unsigned long long physical_memory_total_in_megabytes()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	//DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;
	//DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
	DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
	//DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;	// physical memory used by the system
	//SIZE_T physMemUsedByMe = pmc.WorkingSetSize;	// by current process
	return (totalPhysMem / (1024ULL * 1024));
#else
	struct sysinfo memInfo;

	sysinfo(&memInfo);
	long long totalVirtualMem = memInfo.totalram;
	//Add other values in next statement to avoid int overflow on right hand side...
	totalVirtualMem += memInfo.totalswap;
	totalVirtualMem *= memInfo.mem_unit;
	long long virtualMemUsed = memInfo.totalram - memInfo.freeram;
	//Add other values in next statement to avoid int overflow on right hand side...
	virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
	virtualMemUsed *= memInfo.mem_unit;
	long long totalPhysMem = memInfo.totalram;
	//Multiply in next statement to avoid int overflow on right hand side...
	totalPhysMem *= memInfo.mem_unit;
	long long physMemUsed = memInfo.totalram - memInfo.freeram;
	//Multiply in next statement to avoid int overflow on right hand side...
	physMemUsed *= memInfo.mem_unit;	// total physical memory used by the whole system
	return (totalPhysMem / (1024ULL * 1024));
#endif
}

// Test memory allocation
int TestMemoryAllocation()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	const size_t NUM_TIMES = 1000;
	const size_t SIZE_OF_ARRAY = 100'000'000;
	unsigned char* array_of_pointers[NUM_TIMES]{};
	size_t sum = 0;

	for (size_t i = 0; i < NUM_TIMES; ++i)
	{
		array_of_pointers[i] = new unsigned char[SIZE_OF_ARRAY];
		for (size_t j = 0; j < SIZE_OF_ARRAY; ++j)
			array_of_pointers[i][j] = (unsigned char)j;
		for (size_t j = 0; j < SIZE_OF_ARRAY; ++j)
			sum += array_of_pointers[i][j];
		printf("Allocated array: %zu   sum = %zu\n", i, sum);
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}
	printf("Final sum = %zu\n", sum);

	for (size_t i = 0; i < NUM_TIMES; ++i)
	{
		delete[] array_of_pointers[i];
	}
#endif
	return 0;
}

void print_current_memory_space()
{
	printf("physical memory used = %llu   physical memory total = %llu\n",
		physical_memory_used_in_megabytes(), physical_memory_total_in_megabytes());
}

void test_lazy_memory_allocation()
{
	const size_t SIZE_OF_ARRAY = 10'000'000'000;
	//size_t sum = 0;

	print_current_memory_space();

	unsigned char* my_array = new unsigned char[SIZE_OF_ARRAY];

	print_current_memory_space();

	for (size_t i = 0; i < (SIZE_OF_ARRAY / 2); ++i)
		my_array[i] = (unsigned char)i;

	print_current_memory_space();

	for (size_t i = (SIZE_OF_ARRAY / 2); i < SIZE_OF_ARRAY; ++i)
		my_array[i] = (unsigned char)i;

	print_current_memory_space();

	delete[] my_array;

	print_current_memory_space();
}
