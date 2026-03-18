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
#include <math.h>
#include <chrono>
#include <random>
#include <hip/hip_runtime.h>
#include <hip/hip_fp16.h>
#include "reference.h"

#define H2F(input) static_cast<accscalar_t>(input)
#define F2H(input) static_cast<scalar_t>(input)

#define DEVICE_LINEAR_GET(A, INDEX) A[INDEX]
#define DEVICE_BIAS_GET(A, INDEX) A[INDEX]

template<typename T>
inline

template <typename scalar_t, typename accscalar_t, typename index_type>

