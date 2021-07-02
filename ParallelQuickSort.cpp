/*
Copyright (C) 2019 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.

SPDX-License-Identifier: MIT
*/

// avoid Windows macros
#define NOMINMAX

#include <algorithm>
#include <cfloat>
#include <iostream>
#include <random>
#include <vector>
#include <thread>
#include <tbb/tbb.h>

struct DataItem {
    int id;
    double value;
    DataItem(int i, double v) : id{ i }, value{ v } {}
};

using QSVector = std::vector<unsigned>;

void serialQuicksortFromBook(QSVector::iterator b, QSVector::iterator e);
void serialQuicksort(QSVector::iterator b, QSVector::iterator e);

// Has a performance issue for pre-sorted arrays, possibly O(N^2) performance for pre-sorted input data
void parallelQuicksortFromBook(QSVector::iterator b, QSVector::iterator e) {
    const int cutoff = 100;

    if (e - b < cutoff) {
        serialQuicksortFromBook(b, e);
    }
    else {
        // do shuffle
        unsigned pivot_value = *b;
        QSVector::iterator i = b, j = e - 1;
        while (i != j) {
            while (i != j && pivot_value < *j ) --j;
            while (i != j && *i <= pivot_value) ++i;
            std::iter_swap(i, j);
        }
        std::iter_swap(b, i);

        // recursive call
        tbb::parallel_invoke(
            [=]() { parallelQuicksortFromBook(b,     i); },
            [=]() { parallelQuicksortFromBook(i + 1, e); }
        );
    }
}

void parallelQuicksort(QSVector::iterator b, QSVector::iterator e) {

    const int cutoff = 100;

    if (e - b < cutoff) {
        serialQuicksort(b, e);
    }
    else {

        // do shuffle
        QSVector::iterator i = b, j = e - 1;
        auto pivot_value = *(i + (j - i) / 2);
        if (*i < pivot_value)
            while (*(++i) < pivot_value);
        if (*j > pivot_value)
            while (*(--j) > pivot_value);

        while (i < j)
        {
            std::iter_swap(i, j);
            while (*(++i) < pivot_value);
            while (*(--j) > pivot_value);
        }
        j++;

        tbb::parallel_invoke(
            [=]() { parallelQuicksort(b, j); },
            [=]() { parallelQuicksort(j, e); }
        );
    }
}

// Has a performance issue for pre-sorted arrays, possibly O(N^2) performance for pre-sorted input data
void serialQuicksortFromBook(QSVector::iterator b, QSVector::iterator e) {
    if (b >= e) return;

    // do shuffle
    unsigned pivot_value = *b;
    QSVector::iterator i = b, j = e - 1;
    while (i != j) {
        while (i != j && pivot_value < *j ) --j;
        while (i != j && *i <= pivot_value) ++i;
        std::iter_swap(i, j);
    }
    std::iter_swap(b, i);

    // recursive call
    serialQuicksortFromBook(b,     i);
    serialQuicksortFromBook(i + 1, e);
}

// ADAPTED FROM:https://stackoverflow.com/questions/53722004/generic-quicksort-implemented-with-vector-and-iterators-c
void serialQuicksort(QSVector::iterator b, QSVector::iterator e) {

    if ((e - b) < 2)
        return;

    // do shuffle
    QSVector::iterator i = b, j = e - 1;
    auto pivot_value = *(i + (j - i) / 2);
    if (*i < pivot_value)
        while (*(++i) < pivot_value);
    if (*j > pivot_value)
        while (*(--j) > pivot_value);

    while (i < j)
    {
        std::iter_swap(i, j);
        while (*(++i) < pivot_value);
        while (*(--j) > pivot_value);
    }
    j++;

    // recursive call
    serialQuicksort(b, j);
    serialQuicksort(j, e);
}
// FROM:https://stackoverflow.com/questions/53722004/generic-quicksort-implemented-with-vector-and-iterators-c
// Generic implementation
template <typename I>
void serialQuickSort2(I beg, I end)
{
    if (end - beg < 2)
        return;
    I lft(beg);
    I rgt(end - 1);
    auto pvt = *(lft + (rgt - lft) / 2);
    if (*lft < pvt)
        while (*++lft < pvt);
    if (*rgt > pvt)
        while (*--rgt > pvt);
    while (lft < rgt)
    {
        std::iter_swap(lft, rgt);
        while (*++lft < pvt);
        while (*--rgt > pvt);
    }
    rgt++;
    serialQuickSort2(beg, rgt);
    serialQuickSort2(rgt, end);
}

static QSVector makeQSData(int N) {
    QSVector v;
#if 0
    std::default_random_engine g;
    std::uniform_real_distribution<unsigned> d(0, std::numeric_limits<unsigned>::max());

    for (int i = 0; i < N; ++i)
        v.push_back(d(g));
#endif
    std::random_device rd;

    for (int i = 0; i < N; ++i)
        v.push_back(static_cast<unsigned>(rd()));

    return v;
}

static bool checkIsSorted(const QSVector& v) {
    double max_value = std::numeric_limits<unsigned>::min();
    for (auto e : v) {
        if (e < max_value) {
            std::cerr << "Sort FAILED" << std::endl;
            return false;
        }
        max_value = e;
    }
    return true;
}

static void warmupTBB() {
    tbb::parallel_for(0, (int)std::thread::hardware_concurrency(), [](int) {
        tbb::tick_count t0 = tbb::tick_count::now();
        while ((tbb::tick_count::now() - t0).seconds() < 0.01);
        });
}

int main_quicksort() {
    std::cout << std::endl << "Parallel Quicksort" << std::endl;
    const int N = 10000000;

    QSVector v = makeQSData(N);

    warmupTBB();
    double parallel_time = 0.0;
    {
        tbb::tick_count t0 = tbb::tick_count::now();
        parallelQuicksort(v.begin(), v.end());
        //parallelQuicksort(v.begin(), v.end());                      // see if sorting a pre-sorted array runs infinitely as it does in C#
        //serialQuicksortFromBook(v.begin(), v.end());
        //serialQuicksort(v.begin(), v.end());
        parallel_time = (tbb::tick_count::now() - t0).seconds();
        std::cout << "parallel_time == " << parallel_time << " seconds" << std::endl;
        std::cout << "Done with the first pass of serialQuicksort" << std::endl;
        t0 = tbb::tick_count::now();
        //serialQuicksortFromBook(v.begin(), v.end());
        //serialQuicksort(v.begin(), v.end());
        parallel_time = (tbb::tick_count::now() - t0).seconds();
        std::cout << "parallel_time == " << parallel_time << " seconds" << std::endl;
        std::cout << "Done with the second pass of serialQuicksort" << std::endl;
        if (!checkIsSorted(v)) {
            std::cerr << "ERROR: tbb sorted list out-of-order" << std::endl;
        }
    }

    //std::cout << "parallel_time == " << parallel_time << " seconds" << std::endl;
    return 0;
}

