#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
using std::min;
using std::max;

// --- from warpsort.cu ---
//
// Copyright 2004-present Facebook. All Rights Reserved.
//

#include <assert.h>
#include <chrono>

namespace facebook { namespace cuda {

/** @file

    Sorting routines for in-register warp shuffle sorting. Can
    handle arbitrary sizes via recursive decomposition as long as you
    have enough registers allocated for the problem, though efficiency
    dies as you get significantly larger than 128.

    Template instantiations are provided for input sizes 1-128 (`4 * warpSize`).
*/

// Retrieves 16 values from the warp-held arr[N] from `index` to `index
// + 15`, considering it as an index across the warp threads.
// e.g., returns arr[index / warpSize] from lanes
// L = 16 * ((index / halfWarpSize) % 2) to L + 15 inclusive.
template <int N, typename T>
T
getMulti(const T arr[N], int index, T val) {
  // Only lanes 0-15 load new data
  const int bucket = index / WARP_SIZE;

  // 0 if we are reading using lanes 0-15, 1 otherwise
  const int halfWarp = (index / HALF_WARP_SIZE) & 0x1;

  T load = RegisterIndexUtils<T, N>::get(arr, bucket);
  T shift = shfl_down(load, HALF_WARP_SIZE);

  load = halfWarp ? shift : load;
  // Destination index of arr
  const int bucket = index / WARP_SIZE;

  // Which half warp threads participate in the write?
  // 0 if arr[bucket]:lane 0-15 = val:lane 0-15
  // 1 if arr[bucket]:lane 16-31 = val:lane 0-15
  // `val` always comes from lanes 0-15
  const int halfWarp = (index / HALF_WARP_SIZE) & 0x1;

  // If we are writing to lanes 16-31, we need to get the value from
  // lanes 0-15
  T shift = shfl_up(val, HALF_WARP_SIZE);
  val = halfWarp ? shift : val;

  // Are we in the half-warp that we want to be in?
}

// Performs the merging step of a merge sort within a warp, using
// registers `a[M]` and `b[N]` as the two sorted input lists,
// outputting a sorted `dst[M + N]`. All storage is in registers
// across the warp, and uses warp shuffles for data manipulation.
template <typename T, typename Comparator, int M, int N>

#define STATIC_FLOOR(N, DIV) (int) (N / DIV)
#define STATIC_CEIL(N, DIV) (int) ((N + DIV - 1) / DIV)

// Recursive merging of N sorted lists into 1 sorted list
template <typename T, typename Comparator, int N>
struct Merge {
};

#undef STATIC_FLOOR
#undef STATIC_CEIL

// Base case: 1 list requires no merging
template <typename T, typename Comparator>
struct Merge<T, Comparator, 1> {
};

template <typename T, typename Comparator, int N>

// Sort keys only
template <typename T, typename Comparator, int N>

// Sort keys, writing the sorted keys and the original indices of the
// sorted keys out into two different arrays
template <typename T, typename IndexType, typename Comparator, int N>

// Sort a key/value pair in two different arrays
template <typename K, typename V, typename Comparator, int N>

// Sort keys only; returns true if we could handle an array of this size
template <typename T, typename Comparator>

// Sort keys, writing the sorted keys and the original indices of the
// sorted keys out into two different arrays. Returns true if we could
// handle an array of this size.
template <typename T, typename IndexType, typename Comparator>

// Sort a key/value pair in two different arrays. Returns true if we
// could handle an array of this size.
template <typename K, typename V, typename Comparator>

// Define sort kernels



// Define sort functions called in the main

std::vector<float>
sort(const std::vector<float>& data, double &time) {
  const size_t sizeBytes = data.size() * sizeof(float);

  float* devFloat = NULL;

  float* devResult = NULL;

  dim3 grid(1);
  int outSizes[] = { (int) data.size() };

  auto start = std::chrono::steady_clock::now();

    DeviceTensor<float, 1>(devFloat, dataSizes),
    DeviceTensor<float, 1>(devResult, outSizes));

  auto end = std::chrono::steady_clock::now();
  time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

  std::vector<float> vals(data.size());

  return vals;
}

std::vector<std::pair<float, int> >
sortWithIndices(const std::vector<float>& data, double &time) {
  const size_t sizeBytes = data.size() * sizeof(float);
  const size_t sizeIndicesBytes = data.size() * sizeof(int);

  float* devFloat = NULL;

  float* devResult = NULL;

  int* devIndices = NULL;

  dim3 grid(1);
  int outSizes[] = { (int) data.size() };

  auto start = std::chrono::steady_clock::now();

    DeviceTensor<float, 1>(devFloat, dataSizes),
    DeviceTensor<float, 1>(devResult, outSizes),
    DeviceTensor<int, 1>(devIndices, outSizes));

  auto end = std::chrono::steady_clock::now();
  time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

  std::vector<float> vals(data.size());

             devResult, sizeBytes, cudaMemcpyDeviceToHost);

  std::vector<int> indices(data.size());

             devIndices, sizeIndicesBytes, cudaMemcpyDeviceToHost);

  std::vector<std::pair<float, int> > result;

  return result;
}

} } // namespace
