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
#include <string.h>

#define VECTOR_SIZE (1024*1024)




// --- from shmem_kernels.cu ---
/**
 * shmem_kernels.cu: This file is part of the gpumembench suite.
 *
 * Contact: Elias Konstantinidis <ekondis@gmail.com>
 **/

#include <chrono> // timing
#include <stdio.h>

using namespace std::chrono;

#define TOTAL_ITERATIONS (1024)
#define BLOCK_SIZE 256

// shared memory swap operation







// --- from shmem_kernels.h ---
/**
 * shmem_kernels.h: This file is part of the gpumembench micro-benchmark suite.
 *
 * Contact: Elias Konstantinidis <ekondis@gmail.com>
 **/

#pragma once

void shmembenchGPU(double*, const long, const int);

