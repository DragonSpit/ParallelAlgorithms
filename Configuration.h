// Controls configuration of ParallelAlgorithms
// Supports for Windows, either Microsoft PPL or Intel TBB
// Supports for Linux, Intel TBB

#pragma once

//#define USE_TBB      // Uncomment for Windows to use Intel TBB

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
  #ifndef USE_TBB
    #define USE_PPL
    #include <ppl.h>
  #else
    #include "tbb/tbb.h"
    #include <tbb/parallel_invoke.h>
    #include <tbb/task_group.h>
#endif
#else
  #define USE_TBB
  #include "tbb/tbb.h"
  #include <tbb/parallel_invoke.h>
  #include <tbb/task_group.h>
#endif
