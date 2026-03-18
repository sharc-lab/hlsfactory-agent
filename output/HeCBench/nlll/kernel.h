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

// --- from main.cu ---
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <random>

template <typename scalar_t, typename accscalar_t, 
          typename index_t, int NLL_LOSS_THREADS>

template <typename scalar_t, typename index_t, int GPU_THREADS>

template <typename scalar_t, typename index_t>

