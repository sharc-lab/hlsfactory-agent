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

// --- from kernels.cu ---
#include <stdlib.h>
#include <chrono>
#include <hip/hip_runtime.h>
#include "mv.h"

// sparse matrix vector multiply using the CSR format

// vector sparse matrix vector multiply using the CSR format
template <int BS>

// dense matrix vector multiply



// Reference
// https://github.com/ROCm/rocm-blogs/blob/release/blogs/high-performance-computing/spmv/part-1/examples/vector_csr.cpp

