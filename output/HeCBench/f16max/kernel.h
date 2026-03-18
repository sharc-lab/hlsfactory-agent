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
/* Reference: https://x.momo86.net/en?p=113 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>

#define NUM_OF_BLOCKS 1048576
#define NUM_OF_THREADS 256



template <typename T>


// compute the maximum of two values
