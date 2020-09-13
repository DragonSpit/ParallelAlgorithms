# ParallelAlgorithms

High Performance C++ Algorithms: parallel and sequential. Working VisualStudio 2019 solution is included with all supporting code and usage examples.

- Parallel Merge for merging two arrays of any data type supporting comparison. Described in blog https://duvanenko.tech.blog/2018/01/14/parallel-merge/
- Parallel Merge Sort for sorting arrays of any data type supporting comparison. Described in blog https://duvanenko.tech.blog/2018/01/13/parallel-merge-sort/
- Parallel Sort from Standard C++17. Described in blog https://blogs.msdn.microsoft.com/vcblog/2018/09/11/using-c17-parallel-algorithms-for-better-performance/
- LSD Radix Sort for arrays of unsigned long's. Described in blog https://blogs.msdn.microsoft.com/vcblog/2018/09/11/using-c17-parallel-algorithms-for-better-performance/

Benchmark Results of the following C++17 algorithms on Ubuntu 20.04 using g++, sorting an array of 10 Million unsigned integers:
- single core: sort(sorted.begin(), sorted.end())
- multi-core: sort(std::execution::par_unseq, sorted.begin(), sorted.end())

*Algorithm*|*Random*|*Presorted*|*Constant*|*Description*
--- | --- | --- | --- | ---
sort single core |14|72|| 48-core Intel Xeon, with hyperthreading (96 vCPUs)
sort multi-core |625|1,333|| 48-core Intel Xeon, with hyperthreading (96 vCPUs)

the units in the table above are Millions of unsigned integers per second.

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=LDD8L7UPAC7QL)
