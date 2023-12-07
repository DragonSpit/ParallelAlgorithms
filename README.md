# ParallelAlgorithms

High Performance Parallel (and Sequential) C++ Algorithms.

### Multi-Core Parallel Sorting Algorithms:

*Algorithm*|*Random*|*Presorted*|*Constant*|*Description*
--- | --- | --- | --- | ---
LSD Radix Sort        |2338|2297|2255| 48-core AWS C7i.24xlarge
LSD Radix Sort        | 952| 831| 846| 14-core Intel i7-12700H
Merge Sort            | 695| 946|1954| 48-core AWS C7i.24xlarge
Merge Sort            | 174| 275| 617| 14-core Intel i7-12700H
Merge Sort (in-place) |    |    |    | 48-core AWS C7i.24xlarge
Merge Sort (in-place) |    |    |    | 14-core Intel i7-12700H

The above performance is in millions of unsigned 32-bit integers/second when sorting an array of 100 million elements.
Benchmarks ran on Linux.

### High Performance Single-Core Sequential Algorithms

*Algorithm*|*Random*|*Presorted*|*Constant*|*Description*
--- | --- | --- | --- | ---
LSD Radix Sort (two phase) |153|139|159| 14-core Intel i7-12700H
Merge Sort                 | 12|136|177| 14-core Intel i7-12700H
Merge Sort (in-place)      | 12| 97|296| 14-core Intel i7-12700H
MSD Radix Sort (in-place)  |||| 14-core Intel i7-12700H

LSD Radix Sort single-core with two additional tools:
- novel two-phase implementation reduces passes over the array to (1 + D), where D is the number of digits
- de-randomization of writes

Windows support:
- VisualStudio 2022 Microsoft compiler and Intel's OneAPI compiler. Solution is included.
- Intel's Threading Building Blocks (TBB) and Microsoft's Parallel Patterns Library (PPL)
- C++17

Linux support:
- g++ using Intel's Threading Building Blocks (TBB)
- C++20

[Benchmarks of C++ Standard Parallel Algorithms (STL)](https://duvanenko.tech.blog/2023/05/21/c-parallel-stl-benchmark/) are provided, with benchmark code in [ParallelSTL](https://github.com/DragonSpit/ParallelSTL) repository, which builds and runs on Linix and Windows.

## Other Algorithms

Sorting algorithms provided in this repository:
- Single-core LSD Radix Sort: Two Phase
- Multi-core Parallel LSD Radix Sort: over 2 GigaElements/second
- Multi-core Parallel Merge Sort
- Single-core In-Place Merge Sort
- Multi-core Parallel In-Place Merge Sort
- Single-core MSD Radix Sort (in-place)

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
- Parallel Merge for merging two arrays of any data type supporting comparison. Described in [blog](https://duvanenko.tech.blog/2018/01/14/parallel-merge/)
- Parallel Merge Sort for sorting arrays of any data type supporting comparison. Described in [blog](https://duvanenko.tech.blog/2018/01/13/parallel-merge-sort/)
- Novel LSD Radix Sort (two-phase). Described in [blog](https://duvanenko.tech.blog/2019/02/27/lsd-radix-sort-performance-improvements/)
- Parallel Sort from Standard C++17. Described in [blog](https://blogs.msdn.microsoft.com/vcblog/2018/09/11/using-c17-parallel-algorithms-for-better-performance/)
- LSD Radix Sort for arrays of unsigned long's. Described in [blog](https://blogs.msdn.microsoft.com/vcblog/2018/09/11/using-c17-parallel-algorithms-for-better-performance/)


[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=LDD8L7UPAC7QL)
