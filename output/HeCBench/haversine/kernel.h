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

// --- from distance.cu ---




// --- from main.cu ---
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>





// --- from distance.h ---
#ifndef DIST_H
#define DIST_H

#include <stdio.h>
#include <math.h>
#include <chrono>

#if defined __CUDACC__

#elif defined __HIPCC__
#include <hip/hip_runtime.h>

#elif defined _OPENMP
typedef struct __attribute__((__aligned__(16)))
{
  double x, y, z, w;
} double4;

#else
#include <sycl/sycl.hpp>

#endif

#define DEGREE_TO_RADIAN  M_PI / 180.0
#define RADIAN_TO_DEGREE  180.0 / M_PI
#define EARTH_RADIUS_KM   6371.0

#endif
