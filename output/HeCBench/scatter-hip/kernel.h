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
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <hip/hip_runtime.h>
#include "TensorInfo.h"
#include "reducer.h"

#define THREADS 256
#define BLOCKS(N) (N + THREADS - 1) / THREADS

#define CHECK_HIP(func)                                                       \
{                                                                             \
    hipError_t status = func;                                                 \
}

template <typename scalar_t, ReductionType REDUCE>

template<typename scalar_t, ReductionType REDUCE>

