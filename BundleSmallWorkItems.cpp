//#include "oneapi/tbb/tbbmalloc_proxy.h"     // Intel's scalable memory allocator
#include <iostream>
#include <tbb/task_group.h>
#include <vector>
#include <functional>
#include <random>
#include <algorithm>
#include <chrono>
#include <ratio>
#include <execution>


using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;
using std::random_device;
using std::sort;
using std::vector;

#include "ParallelMergeSort.h"

void print_results(const char* const tag, const vector<bool>& in_array,
    high_resolution_clock::time_point startTime,
    high_resolution_clock::time_point endTime)
{
    printf("%s: size = %zu  Lowest: %d Highest: %d Time: %fms\n", tag, in_array.size(), in_array.front(), in_array.back(),
        duration_cast<duration<double, milli>>(endTime - startTime).count());
}

// Take the number_of_items of size_of_each_item:
// - run them sequentially
// - run them in parallel
// - bundle multiple items and run these bundles in parallel
// Bundle 1, 2, 3, ..., small work items, such as std::all_of() 
int bundling_small_work_items_benchmark(size_t max_bundle_size = 20, size_t number_of_items = 1000, size_t size_of_item = 100)
{
    std::vector<int>                    item(size_of_item, 2);
    std::vector< std::vector<int> >     all_data(number_of_items, item);         // all_data[number_of_items][size_of_item]
    std::vector< bool >                 return_values(number_of_items, false);
    std::vector<std::function<void()> > bundle_of_functions;                     // vector of function calls which take no arguments and return void
    std::vector< std::vector<std::function<void()> > >     all_bundles(number_of_items, bundle_of_functions);         // all_bundles[number_of_items][?]

    high_resolution_clock::time_point startTime, endTime;

    if (number_of_items == 0)
        return 0;

    // Run and time Sequential algorithms
    startTime = high_resolution_clock::now();
    for (size_t i = 0; i < number_of_items; i++)
    {
        return_values[i] = std::all_of(all_data[i].begin(), all_data[i].end(), [](int j) { return j == 2; });
    }
    endTime = high_resolution_clock::now();

    bool combined_return_values = return_values[0];
    for (size_t i = 1; i < number_of_items; i++)
    {
        combined_return_values = combined_return_values && return_values[i];
    }
    if (combined_return_values)
        print_results("Sequential std::all_of", return_values, startTime, endTime);
    else
        printf("Error: Sequential std::all_of failed!\n");

    // Parallel execute and time many sequential algorithms
    return_values.assign(return_values.size(), false);
    startTime = high_resolution_clock::now();

#if defined(USE_PPL)
    Concurrency::task_group g;
#else
    tbb::task_group g;
#endif

    for (size_t i = 0; i < number_of_items; i++)
    {
        g.run([=, &return_values] {													// important to not pass by reference, as all tasks will then get the same/last value
            return_values[i] = std::all_of(all_data[i].begin(), all_data[i].end(), [](int j) { return j == 2; });
            });
    }
    g.wait();
    endTime = high_resolution_clock::now();

    combined_return_values = return_values[0];
    for (size_t i = 1; i < number_of_items; i++)
    {
        combined_return_values = combined_return_values && return_values[i];
    }
    if (combined_return_values)
        print_results("Parallel   std::all_of", return_values, startTime, endTime);
    else
        printf("Error: Parallel   std::all_of failed!\n");

    // Parallel execute and benchmark bundles of sequential algorithms
    for (size_t bundle_size = number_of_items; bundle_size > (number_of_items / 512); bundle_size /= 2)
    {
        return_values.assign(return_values.size(), false);
        for (size_t n = 0; n < number_of_items; n++)
            all_bundles[n].clear();
        size_t number_of_full_bundles = number_of_items / bundle_size;  // i.e. do only full bundles and not the partial bundle at the end
        printf("number_of_full_bundles = %zu  number_of_items = %zu  bundle_size = %zu\n", number_of_full_bundles, number_of_items, bundle_size);
        size_t ci = 0;                                                  // current item
        startTime = high_resolution_clock::now();
        for (size_t b = 0; b < number_of_full_bundles; b++)     // create all bundles
        {
            //printf("b = %zu\b", b);
            for (size_t bs = 0; bs < bundle_size; bs++, ci++)   // create a single bundle of bundle_size items
            {
                //printf("bs = %zu   ci = %zu\b", bs, ci);
                all_bundles[b].push_back([&, ci, b] { return_values[b] = std::all_of(all_data[ci].begin(), all_data[ci].end(), [](int j) { return j == 2; }); });
            }
        }
        endTime = high_resolution_clock::now();
        print_results("Time to create all bundles", return_values, startTime, endTime);

        startTime = high_resolution_clock::now();
        // Run all bundles in parallel
        for (size_t b = 0; b < number_of_full_bundles; b++)
        {
            g.run([=, &all_bundles] {
                for (const auto& func : all_bundles[b]) {
                    func(); // Execute each function within this bundle sequentially
                }
                });
        }
        g.wait();
        endTime = high_resolution_clock::now();
        print_results("Time to run all bundles", return_values, startTime, endTime);

        combined_return_values = return_values[0];
        for (size_t i = 1; i < number_of_full_bundles; i++)
        {
            combined_return_values = combined_return_values && return_values[i];
        }
        if (combined_return_values)
            print_results("Parallel bundles of std::all_of", return_values, startTime, endTime);
        else
            printf("Error: Parallel bundles of std::all_of failed!\n");
    }

    return 0;
}


