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
#include <chrono>
#include <cstdio>
#include <hip/hip_runtime.h>
#include "kiss.cuh"

template <class T, class Rng>

template<class T, class Rng>
inline

// generation of gigabytes of uniform random values

