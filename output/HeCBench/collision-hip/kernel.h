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

// --- from main.cu ---
//
// Copyright 2004-present Facebook. All Rights Reserved.
//

#include <stdlib.h>
#include <assert.h>
#include <iterator>
#include <vector>
#include <chrono>
#include <hip/hip_runtime.h>

using namespace std;

#define WARP_SIZE 32

/// A simple pair type for CUDA device usage
template <typename K, typename V>
struct Pair {


  inline bool
  operator==(const Pair<K, V>& rhs) const {
    return !operator==(rhs);
  }

  inline bool
  operator<(const Pair<K, V>& rhs) const {
    return (k > rhs.k) || ((k == rhs.k) && (v > rhs.v));
  }

  K k;
  V v;
};

/**
   Extract a single bit at `pos` from `val`
*/


/**
   Return the current thread's lane in the warp
*/

template <typename T>
struct GreaterThan {
};

template <typename T>
struct LessThan {
};

template <typename T>
inline T
shfl_xor(const T val, int laneMask, int width = WARP_SIZE) {
  return Pair<K, V>(__shfl_xor(p.k, laneMask, width),
  T y = shfl_xor(x, mask);
  return Comparator::compare(x, y) == dir ? y : x;
}

/// Defines a bitonic sort network to exchange 'V' according to
/// `SWAP()`'s compare and exchange mechanism across the warp, ordered
/// according to the comparator `comp`. In other words, if `comp` is
/// `GreaterThan<T>`, then lane 0 will contain the highest `val`
/// presented across the warp
///
/// See also 
/// http://on-demand.gputechconf.com/gtc/2013/presentations/S3174-Kepler-Shuffle-Tips-Tricks.pdf
template <typename T, typename Comparator>

/// Determine if two warp threads have the same value (a collision).
template <typename T>

/// Determine if two warp threads have the same value (a collision),
/// and returns a bitmask of the lanes that are known to collide with
/// other lanes. Not all lanes that are mutually colliding return a
/// bit; all lanes with a `1` bit are guaranteed to collide with a
/// lane with a `0` bit, so the mask can be used to serialize
/// execution for lanes that collide with others.
/// (mask | (mask >> 1)) will yield all mutually colliding lanes.
template <typename T>

int hasDuplicate[32];


unsigned int duplicateMask;


vector<int> checkDuplicates(const vector<int>& v) {
  int* devSet = NULL;
  hipMalloc(&devSet, v.size() * sizeof(int));
  hipMemcpy(devSet, v.data(), v.size() * sizeof(int), hipMemcpyHostToDevice);

  hipLaunchKernelGGL(checkDuplicates, 1, 32, 0, 0, v.size(), devSet);

  vector<int> hasDuplicates(32, false);
  hipMemcpyFromSymbol(hasDuplicates.data(), HIP_SYMBOL(hasDuplicate), sizeof(int) * 32, 0,
                       hipMemcpyDeviceToHost);
  hipFree(devSet);

  return hasDuplicates;
}




