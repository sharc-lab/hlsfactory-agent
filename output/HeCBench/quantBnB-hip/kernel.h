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
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>
#include <hip/hip_runtime.h>
#include "block_load.h"
#include "block_store.h"
#include "code.h"

template <int STOCHASTIC>
uint8_t
dQuantize(float* smem_code, const float rand, float x)
{
    int pivot = 127;
    int upper_pivot = 255;
    int lower_pivot = 0;

    float lower = -1.0f;
    float upper = 1.0f;

    float val = smem_code[pivot];

    if(upper_pivot == 255)
        upper = smem_code[upper_pivot];
    if(lower_pivot == 0)
        lower = smem_code[lower_pivot];

    else
    {
      else
      {
        float dist_to_lower = fabsf(lower-x);
        float dist_full = val-lower;
        if(rand >= dist_to_lower/dist_full) return lower_pivot;
        else return pivot;
      }
    }
}

#define NUM 4

template<int TH, int BLOCK_SIZE>

