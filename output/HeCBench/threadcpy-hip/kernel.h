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
#include <iostream>
#include <chrono>
#include <cstring>
#include <hip/hip_runtime.h>

#define GPU_CHECK(x) do { \
  hipError_t err = x; \
} while (0)

template <typename T, int VEC_SIZE>

template <typename T, int vec_size>

template <int vec_size, typename scalar_t>

