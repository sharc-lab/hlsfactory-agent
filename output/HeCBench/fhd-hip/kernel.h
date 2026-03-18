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
#include <hip/hip_runtime.h>

#define CHUNK_S 4096

typedef struct {
  float x, y, z;
} kdata;

kdata k[CHUNK_S];


