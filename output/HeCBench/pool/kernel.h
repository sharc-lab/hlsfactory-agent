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
#include <new>
#include <string>

#define HOSTDEVICE // thread block size
#define BSIZE 256

template <class T>
class AvgPoolGrad {
  public:
};

template <class T>
class MaxPoolGrad {
  public:
};


template <typename PoolProcess, typename T,
          int ksize_height,
          int ksize_width,
          int stride_height,
          int stride_width,
          int padding_height,
          int padding_width,
          bool exclusive>

