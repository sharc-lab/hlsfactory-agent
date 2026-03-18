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

// --- from sptrsv_syncfree.cu ---
#ifndef _SPTRSV_SYNCFREE_
#define _SPTRSV_SYNCFREE_

#include <chrono>
#include <hip/hip_runtime.h>
#include "sptrsv.h"

// reference
// https://stackoverflow.com/questions/32341081/how-to-have-atomic-load-in-cuda

// addr must be aligned properly.

// addr must be aligned properly.



#endif
