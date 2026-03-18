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

// --- from kernels.cu ---
#include "hip/hip_runtime.h"
#include <stdio.h>
#include <chrono>
#include "utils.h"

// Thread block size
#define GROUP_SIZE 256


inline double4 operator+(double4 a, double4 b)
{
  return {s,s,s,s,s,s,s,s};
}

// Calculates equivalent distribution 

// convert_int8() may be language specific



