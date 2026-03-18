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
#include <cstdio>
#include <chrono>
#include <utility>  // std::pair
#include <hip/hip_runtime.h>
#include <hiprand/hiprand_kernel.h>

// philox generates 128 bits of randomness at a time. 
// Kernel uses this explicitly by putting suitably transformed result into float4
// for all members of float4 to be consumed UNROLL has to be 4. Don't change!
const int UNROLL = 4;

template <typename scalar_t,
          typename accscalar_t,
          typename IndexType,
          typename mask_t>

// aligned vector generates vectorized load/store
template<typename scalar_t, int vec_size>

template <typename scalar_t,
          typename accscalar_t,
          typename IndexType,
          typename mask_t,
          int VEC>

