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

#define GPU_THREADS 256

#define KERNEL_LOOP(index, range) \
   for (int index = _bid_x * BLOCK_DIM_X + _tid_x;  \
            index < (range); index += BLOCK_DIM_X * GRID_DIM_X) 

template <typename T>

template <typename T>

template<typename T>

