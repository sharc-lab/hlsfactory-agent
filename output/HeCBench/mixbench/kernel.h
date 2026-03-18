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
/**
 * This file is the modified read-only mixbench GPU micro-benchmark suite.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <chrono>

#define VECTOR_SIZE (8*1024*1024)
#define granularity (8)
#define fusion_degree (4)
#define seed 0.1f



