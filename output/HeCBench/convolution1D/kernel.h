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
/*
  Reference
  Chapter 7 in Programming massively parallel processors,
  A hands-on approach (D. Kirk and W. Hwu)
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>

#define GPU_CHECK(x) do { \
    cudaError_t err = x; \
} while (0)

#define MAX_MASK_WIDTH 10
#define MAX_BLOCK_SIZE 1024

template<typename T>
T mask [MAX_MASK_WIDTH];

template<typename T>

template<typename T>

template<typename T>

template <typename T>

template <typename T>

