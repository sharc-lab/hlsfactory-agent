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

// --- from kernels.cu ---
#include <stdlib.h>
#include <chrono>
#include <hip/hip_runtime.h>
#include <hipsparse/hipsparse.h>
#include "mv.h"

// dense matrix vector multiply


#define CHECK_HIPSPARSE(func)                                                   \
{                                                                              \
    hipsparseStatus_t status = (func);                                         \
}


