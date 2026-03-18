#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
using std::min;
using std::max;

// CUDA thread/block dimension constants (adjust for your workload)
#ifndef BLOCK_DIM_X
#define BLOCK_DIM_X 256
#endif
#ifndef GRID_DIM_X
#define GRID_DIM_X 1
#endif

// --- from main.cu ---
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <hip/hip_runtime.h>

#define threadsPerBlock  256


#define FOR_KERNEL_LOOP(i, lim)                                      \

template <typename input_t, typename IndexType>
static IndexType
getBin(input_t v, input_t minvalue, input_t maxvalue, IndexType nbins)
{
  IndexType bin = (v - minvalue) * nbins / (maxvalue - minvalue);
  // (only applicable for histc)
  // while each bin is inclusive at the lower end and exclusive at the higher,
  // i.e. [start, end) the last bin is inclusive at both, i.e. [start, end], in
  // order to include maxvalue if exists therefore when bin == nbins, adjust bin
  // to the last bin
  if (bin == nbins) bin--;
  return bin;
}

// Kernel for computing the histogram of the input
template <typename output_t,
          typename input_t,
          typename IndexType,
          DeviceMemoryType MemoryType>

#define HANDLE_CASE(MEMORY_TYPE, SHARED_MEM)                 \
  auto start = std::chrono::steady_clock::now();             \
  for (int i = 0; i < repeat; i++)                           \
  bincount<                                                  \
      output_t,                                              \
      input_t,                                               \
      IndexType,                                             \

      d_output,                                              \
      d_input,                                               \
      nbins,                                                 \
      input_minvalue,                                        \
      input_maxvalue,                                        \
      input_size,                                            \
      output_size);                                          \
  hipDeviceSynchronize();                                   \
  auto end = std::chrono::steady_clock::now();               \
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count(); \

/*
  Calculate the frequency of the input values.
  3 implementations based of input size and memory usage:
    case: sufficient shared mem
        SHARED: Each block atomically adds to it's own **shared** hist copy,
        then atomically updates the global tensor.
    case: insufficient shared memory
        GLOBAL: all threads atomically update to a single **global** hist copy.
 */
template <typename output_t, typename input_t, typename IndexType>

