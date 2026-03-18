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
#ifndef BLOCK_DIM_Y
#define BLOCK_DIM_Y 1
#endif
#ifndef GRID_DIM_Y
#define GRID_DIM_Y 1
#endif

// --- from main.cu ---
#include <iostream>
#include <limits>
#include <cmath>
#include <chrono>

using namespace std;

#define BLOCK_SIZE 16

/**
 * Each element of the product matrix c[i][j] is computed from a unique row and
 * column of the factor matrices, a[i][k] and b[k][j]
 */

// Matrix size constants.
constexpr int m_size = 512 * 8;  // Must be a multiple of 8.
constexpr int M = m_size / 8;
constexpr int N = m_size / 4;
constexpr int K = m_size / 2;

#include "verify.cpp"

template <int m, int n, int k>

