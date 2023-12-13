# ParallelAlgorithms

High Performance Parallel (and Sequential) C++ Algorithms, which accompany "[Practical Parallel Algorithms in C++ and C#](https://www.amazon.com/Practical-Parallel-Algorithms-Sorting-Multicore-ebook/dp/B0C3TZPRKZ/ref=sr_1_1?crid=3P7Q0RUP8OBXB&keywords=duvanenko&qid=1702488919&sprefix=duvanenko%2Caps%2C95&sr=8-1)" book.

### Multi-Core Parallel Sorting Algorithms:

*Algorithm*|*Random*|*Presorted*|*Constant*|*Description*
--- | --- | --- | --- | ---
LSD Radix Sort        |2865|2907|4769| 48-core AWS C7a.24xlarge (AMD)
LSD Radix Sort        |2338|2297|2255| 48-core AWS C7i.24xlarge (Intel)
LSD Radix Sort        | 952| 831| 846| 14-core Intel i7-12700H
Merge Sort            | 695| 946|1954| 48-core AWS C7i.24xlarge (Intel)
Merge Sort            | 174| 275| 617| 14-core Intel i7-12700H
Merge Sort (in-place) | 272| 502| 549| 48-core AWS C7i.24xlarge (Intel)
Merge Sort (in-place) |  90| 234| 339| 14-core Intel i7-12700H

The above performance is in millions of unsigned 32-bit integers/second when sorting an array of 100 million elements.
Benchmarks ran on Linux.

### High Performance Single-Core Sequential Algorithms

*Algorithm*|*Random*|*Presorted*|*Constant*|*Description*
--- | --- | --- | --- | ---
LSD Radix Sort (two phase) |153|139|159| 1-core of Intel i7-12700H
Merge Sort                 | 12|136|177| 1-core of Intel i7-12700H
Merge Sort (in-place)      | 12| 97|296| 1-core of Intel i7-12700H
MSD Radix Sort (in-place)  | 41| 48| 46| 1-core of Intel i7-12700H

LSD Radix Sort single-core with two additional performance tools:
- novel two-phase implementation reduces passes over the array to (1 + D), where D is the number of digits
- de-randomization of writes to bins

## Other Algorithms
Sorting algorithms provided in this repository:
- Single-core LSD Radix Sort: Two Phase
- Multi-core Parallel LSD Radix Sort
- Multi-core Parallel Merge Sort
- Single-core In-Place Merge Sort
- Multi-core Parallel In-Place Merge Sort
- Single-core MSD Radix Sort (in-place)

Windows support:
- VisualStudio 2022 Microsoft compiler and Intel's OneAPI compiler. Solution is included.
- Intel's Threading Building Blocks (TBB) and Microsoft's Parallel Patterns Library (PPL)
- C++17

Linux support:
- g++ using Intel's Threading Building Blocks (TBB)
- C++20

## Building on Ubuntu Linux
To install g++ which supports C++17:
```
sudo apt update
sudo apt upgrade
# reboot the machine
sudo apt install build-essential
```

To update gcc to support c++17 standard, Parallel STL and Intel's Threading Building Blocks (TBB):
```
sudo apt install libtbb-dev
git clone https://github.com/DragonSpit/ParallelAlgorithms.git
cd ParallelAlgorithms
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
Intel's OneAPI C++ compiler is supported by the ParallelAlgorithms.sln VisualStudio 2022 Solution. By default, the solution/project uses Microsoft C++ compiler, which avoid requiring installation of Intel's OneAPI.
Once Intel's OneAPI, which is free, has been installed, select Project/IntelCompiler/UseIntelOneAPICompiler. Some of the algorithms are faster when build with Intel's OneAPI compiler.

## Other Resources
[Benchmarks of C++ Standard Parallel Algorithms (STL)](https://duvanenko.tech.blog/2023/05/21/c-parallel-stl-benchmark/) are provided, with benchmark code in [ParallelSTL](https://github.com/DragonSpit/ParallelSTL) repository, which builds and runs on Linix and Windows.

Blogs:
- [Parallel Merge](https://duvanenko.tech.blog/2018/01/14/parallel-merge/) for merging two arrays of any data type supporting comparison.
- [Parallel Merge Sort](https://duvanenko.tech.blog/2018/01/13/parallel-merge-sort/) for sorting arrays of any data type supporting comparison.
- [Novel LSD Radix Sort (two-phase)](https://duvanenko.tech.blog/2019/02/27/lsd-radix-sort-performance-improvements/).
- [Parallel Sort from Standard C++17](https://blogs.msdn.microsoft.com/vcblog/2018/09/11/using-c17-parallel-algorithms-for-better-performance/).
- [LSD Radix Sort](https://blogs.msdn.microsoft.com/vcblog/2018/09/11/using-c17-parallel-algorithms-for-better-performance/) for arrays of unsigned long's.


[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=LDD8L7UPAC7QL)
