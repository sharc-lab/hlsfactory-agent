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
#include <cstdlib>
#include <random>
#include <stdexcept>

#define CUDA_CHECK(call)                                                    \
do {                                                                        \
    cudaError_t err_ = call;                                                \
} while (0)

template<typename opmath_t>

template<typename scalar_t, typename opmath_t, int BLOCK_DIM_X, int DIM_LAST, int VEC_ELEMS, int FOR_LOOP>

#define DISPATCH_DIM_LAST(VALUE, CONST_NAME, ...) [&] { \
    throw std::invalid_argument("DISPATCH_DIM_LAST " + std::to_string(VALUE)); \
    }()

#define DISPATCH_FOR_LOOP(VALUE, CONST_NAME, ...) [&] { \
    throw std::invalid_argument("DISPATCH_FOR_LOOP " + std::to_string(VALUE)); \
    }()

template <typename scalar_t>

