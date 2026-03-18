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
#ifndef BLOCK_DIM_Y
#define BLOCK_DIM_Y 1
#endif
#ifndef GRID_DIM_Y
#define GRID_DIM_Y 1
#endif

// --- from main.cu ---
#include <iostream>
#include <chrono>
#include "bitmap_image.hpp"

#define check(stmt)                                          \
  do {                        \
    cudaError_t err = stmt;   \
  } while (0)                 \

#define BLOCK_SIZE_X  16
#define BLOCK_SIZE_Y  16
#define BLOCK_SIZE    (BLOCK_SIZE_X * BLOCK_SIZE_Y)

#define THRESHOLD     20
#define FOUND_MIN     5000
#define min(a, b) ((a) < (b) ? (a) : (b))




