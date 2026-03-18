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

typedef unsigned long long int u64Int;
typedef long long int s64Int;

/* Random number generator */
#define POLY 0x0000000000000007UL
#define PERIOD 1317624576693539401L

#define NUPDATE (4 * TableSize)

/* CUDA specific parameters */
#define K1_BLOCKSIZE  256
#define K2_BLOCKSIZE  128




