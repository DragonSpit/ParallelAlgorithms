# ParallelAlgorithms

High Performance C++ Algorithms: parallel and sequential.

Working VisualStudio 2019 solution is included with all supporting code and usage examples.
Works with g++ on Linux. Works with VisualStudio 2019 MSVC compiler, and Intel's DPCPP OneAPI compiler on Windows. Works in WSL - Ubuntu 20.04.
Uses C++17 and Intel's Threading Building Block (TBB) or Microsoft's Parallel Patterns Library (PPL)

Benchmark Results of the following C++17 algorithms on Ubuntu 20.04 using g++, sorting an array of 10 Million unsigned 32-bit integers:
- single core: ```sort(sorted.begin(), sorted.end())```
- multi-core: ```sort(std::execution::par_unseq, sorted.begin(), sorted.end())```

*Algorithm*|*Random*|*Presorted*|*Constant*|*Description*
--- | --- | --- | --- | ---
sort single core |11|35|1667| 6-core Intel i7-9750H, with hyperthreading
sort multi-core |62|152|1639| 6-core Intel i7-9750H, with hyperthreading
sort single core |11|32|1492| 48-core Intel Xeon, with hyperthreading (96 vCPUs)
sort multi-core |93|250|1492| 48-core Intel Xeon, with hyperthreading (96 vCPUs)

the units in the table above are Millions of unsigned longs per second. Pre-sorted array is actually nearly pre-sorted, with every 100-th element being a random value.

Additional sorting algorithms provided are:
- Single-core LSD Radix Sort (Two Phase)
- Multi-core Parallel LSD Radix Sort
- Multi-core Parallel Merge Sort, with simple interfaces (see ParallelAlgorithms namespace)
- Single-core In-Place Merge Sort
- Multi-core Parallel In-Place Merge Sort

*Algorithm*|*Random*|*Presorted*|*Constant*|*Description*
--- | --- | --- | --- | ---
LSD Radix Sort single core (two phase) |145|125|104| 6-core Intel i7-9750H, with hyperthreading
LSD Radix Sort multi-core |285|294|322| 6-core Intel i7-9750H, with hyperthreading
LSD Radix Sort single core |60|64|112| 48-core Intel Xeon, with hyperthreading (96 vCPUs)
LSD Radix Sort multi-core |434|476|714| 48-core Intel Xeon, with hyperthreading (96 vCPUs)
Merge Sort single-core |19|93|114| 6-core Intel i7-9750H, with hyperthreading
Merge Sort multi-core |105|222|243| 6-core Intel i7-9750H, with hyperthreading
Merge Sort multi-core |626|1010|1136| 48-core Intel Xeon, with hyperthreading (96 vCPUs)
In-Place Merge Sort multi-core |58|164|1163| 6-core Intel i7-9750H, with hyperthreading
In-Place Merge Sort multi-core |61|145|1230| 48-core Intel Xeon, with hyperthreading (96 vCPUs)

Parallel Merge Sort is over 6X faster than C++ standard sort, on a 48-core machine, showing better scaling
with more cores. It is also 70% faster on 6-core machine. It is also 4X faster for nearly pre-sorted arrays.

## Scaling of Parallel Radix Sort

*Algorithm*|*Random*|*Presorted*|*Constant*|*Description*
--- | --- | --- | --- | ---
LSD Radix Sort single core |143|128|110| 6-core Intel i7-9750H, with hyperthreading
LSD Radix Sort multi-core |325|294|322| 6-core Intel i7-9750H, with hyperthreading
LSD Radix Sort single core |68|76|108| 48-core Intel Xeon, with hyperthreading (96 vCPUs)
LSD Radix Sort multi-core |434|476|714| 48-core Intel Xeon, with hyperthreading (96 vCPUs)

## Building on Ubuntu 20.04 Linux
To install g++ which supports C++17:
```
sudo apt install build-essential
```

To update gcc to support c++17 standard:
```
sudo apt update
sudo apt install libtbb-dev
```

To build, use g++ command and not gcc. The order of the following arguments matters!
```
g++ ParallelAlgorithms.cpp ParallelMergeSortBenchmark.cpp RadixSortLsdBenchmark.cpp ParallelStdCppExample.cpp ParallelQuickSort.cpp -ltbb -std=c++17 -O3 -o ParallelAlgorithms
```

## Building on Windows Using Intel's OneAPI Compiler
In IntelOneAPI directory, open ParallelAlgorithms.sln VisualStudio 2019 Solution, and build it. This solution is setup to use the Intel Compiler.
Intel Compiler produces higher performance Parallel Merge Sort and LSD Radix Sort, as shown in the following table:

*Algorithm*|*Random*|*Presorted*|*Constant*|*Description*
--- | --- | --- | --- | ---
LSD Radix Sort multi-core |248|240|288| 6-core Intel i7-9750H, with hyperthreading

## Reference Blogs
- Parallel Merge for merging two arrays of any data type supporting comparison. Described in blog https://duvanenko.tech.blog/2018/01/14/parallel-merge/
- Parallel Merge Sort for sorting arrays of any data type supporting comparison. Described in blog https://duvanenko.tech.blog/2018/01/13/parallel-merge-sort/
- Parallel Sort from Standard C++17. Described in blog https://blogs.msdn.microsoft.com/vcblog/2018/09/11/using-c17-parallel-algorithms-for-better-performance/
- LSD Radix Sort for arrays of unsigned long's. Described in blog https://blogs.msdn.microsoft.com/vcblog/2018/09/11/using-c17-parallel-algorithms-for-better-performance/


[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=LDD8L7UPAC7QL)
