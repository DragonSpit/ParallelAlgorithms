# ParallelAlgorithms

High Performance C++ Algorithms: parallel and sequential.

Working VisualStudio 2022 solution is included with all supporting code and usage examples.
Works on Linux using g++. Works on Windows using VisualStudio 2022 Microsoft compiler and Intel's OneAPI compiler. Works in WSL - Ubuntu 20.04.
Uses C++17 and Intel's Threading Building Block (TBB) or Microsoft's Parallel Patterns Library (PPL) on Windows.
Uses C++20 and Intel's TBB on Linux.

[Benchmarks of C++ Standard Parallel Algorithms (STL)](https://duvanenko.tech.blog/2023/05/21/c-parallel-stl-benchmark/) are provided, with benchmark code in [ParallelSTL](https://github.com/DragonSpit/ParallelSTL) repository. It builds and runs on Linix and Windows.

Sorting algorithms provided in this repository:
- Single-core LSD Radix Sort: Two Phase
- Multi-core Parallel LSD Radix Sort: over 2 GigaElements/second
- Multi-core Parallel Merge Sort
- Single-core In-Place Merge Sort
- Multi-core Parallel In-Place Merge Sort
- Single-core MSD Radix Sort (in-place)

*Algorithm*|*Random*|*Presorted*|*Constant*|*Description*
--- | --- | --- | --- | ---
LSD Radix Sort single core (two phase) |145|125|104| 6-core Intel i7-9750H
LSD Radix Sort multi-core |325|294|322| 6-core Intel i7-9750H
LSD Radix Sort single core |68|76|108| 24-core Intel Xeon 8275CL
LSD Radix Sort multi-core |434|476|714| 24-core Intel Xeon 8275CL
Merge Sort single-core |19|93|114| 6-core Intel i7-9750H
Merge Sort multi-core |105|222|243| 6-core Intel i7-9750H
Merge Sort multi-core |626|1010|1136| 24-core Intel Xeon 8275CL
In-Place Merge Sort multi-core |58|139|313| 6-core Intel i7-9750H
In-Place Merge Sort multi-core |179|286|333| 24-core Intel Xeon 8275CL

Parallel Merge Sort is over 6X faster than C++ standard sort, on a 48-core machine, showing better scaling
with more cores. It is also 70% faster on 6-core machine. It is also 4X faster for nearly pre-sorted arrays.

## Scaling of Parallel Radix Sort

*Algorithm*|*Random*|*Presorted*|*Constant*|*Description*
--- | --- | --- | --- | ---
LSD Radix Sort single core |143|128|110| 6-core Intel i7-9750H
LSD Radix Sort multi-core |325|294|322| 6-core Intel i7-9750H
LSD Radix Sort single core |68|76|108| 24-core Intel Xeon 8275CL
LSD Radix Sort multi-core |434|476|714| 24-core Intel Xeon 8275CL

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

To build on WSL Ubuntu, use g++ command and not gcc. The order of the following arguments matters!
```
g++ ParallelAlgorithms.cpp ParallelStdCppExample.cpp RadixSortLsdBenchmark.cpp MemoryUsage.cpp CountingSortParallelBenchmark.cpp SumBenchmark.cpp RadixSortMsdBenchmark.cpp ParallelMergeSortBenchmark.cpp -ltbb -std=c++20 -O3 -o ParallelAlgorithms
```
To build on AWS Ubuntu, use g++ command and not gcc. The order of the following arguments matters!
```
g++ ParallelAlgorithms.cpp ParallelStdCppExample.cpp RadixSortLsdBenchmark.cpp MemoryUsage.cpp CountingSortParallelBenchmark.cpp SumBenchmark.cpp RadixSortMsdBenchmark.cpp ParallelMergeSortBenchmark.cpp -ltbb -std=c++2a -O3 -o ParallelAlgorithms
```
To run it:
```
./ParallelAlgorithms
```
## Building on Windows Using Intel's OneAPI Compiler
In IntelOneAPI directory, open ParallelAlgorithms.sln VisualStudio 2019 Solution, and build it. This solution is setup to use the Intel Compiler.
Intel Compiler produces higher performance Parallel Merge Sort and LSD Radix Sort, as shown in the following table:

*Algorithm*|*Random*|*Presorted*|*Constant*|*Description*
--- | --- | --- | --- | ---
LSD Radix Sort multi-core |248|240|288| 6-core Intel i7-9750H

## Reference Blogs
- Parallel Merge for merging two arrays of any data type supporting comparison. Described in blog https://duvanenko.tech.blog/2018/01/14/parallel-merge/
- Parallel Merge Sort for sorting arrays of any data type supporting comparison. Described in blog https://duvanenko.tech.blog/2018/01/13/parallel-merge-sort/
- Parallel Sort from Standard C++17. Described in blog https://blogs.msdn.microsoft.com/vcblog/2018/09/11/using-c17-parallel-algorithms-for-better-performance/
- LSD Radix Sort for arrays of unsigned long's. Described in blog https://blogs.msdn.microsoft.com/vcblog/2018/09/11/using-c17-parallel-algorithms-for-better-performance/


[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=LDD8L7UPAC7QL)
