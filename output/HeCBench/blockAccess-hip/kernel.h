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
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <hip/hip_runtime.h>
#include "block_load.h"
#include "block_store.h"

#define GPU_CHECK(x) do { \
  hipError_t err = x; \
} while (0)


template<int BLOCKSIZE, int ITEMS_PER_THREAD>

