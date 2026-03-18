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
#include <new>
#include <cmath>
#include <chrono>

#define BLOCK_SIZE 16

#ifdef DOUBLE_PRECISION
  #define SQRT sqrt
  #define FABS fabs
  #define FP double
#else
  #define SQRT sqrtf
  #define FABS fabsf
  #define FP float
#endif

/**
 * Each element of the product matrix c[i][j] is computed from a unique row and
 * column of the factor matrices, a[i][k] and b[k][j]
 */

// Matrix size constants.
constexpr int m_size = 768 * 8;  // Must be a multiple of 8.
constexpr int M = m_size / 8;
constexpr int N = m_size / 4;
constexpr int P = m_size / 2;

#ifdef VERIFY
#endif




// --- from verify.h ---
#ifdef VERIFY
#include <limits>

/**
 * Perform matrix multiplication on host to verify results from device.
 */
bool ValueSame(FP a, FP b) {
  return FABS(a - b) < 1e-5;
}

void VerifyResult(FP (*a_host)[N], FP (*b_host)[P], 
                  FP (*c_host)[P], FP (*c_back)[P]) {
  // Check that the results are correct by comparing with host computing.
  int i, j, k;

  // c_host is initialized to zero.
  for (i = 0; i < M; i++)
    for (j = 0; j < P; j++) c_host[i][j] = (FP)0.0;

  for (i = 0; i < M; i++) {
    for (k = 0; k < N; k++) {
      for (j = 0; j < P; j++) {
        c_host[i][j] += SQRT(a_host[i][k] * b_host[k][j]);
      }
    }
  }
  for (i = 0; i < M; i++) {
    for (j = 0; j < P; j++) {
      FP value = ((FP)1.0 - c_host[i][j]);
      FP gate = (!std::signbit(value));  // std::signbit is a host function
      c_host[i][j] = SQRT(gate * value);
    }
  }

  bool mismatch_found = false;

  // Compare host side results with the result buffer from device side: print
  // mismatched data 5 times only.
  int print_count = 0;

  for (i = 0; i < M; i++) {
    for (j = 0; j < P; j++) {
      if (!ValueSame(c_back[i][j], c_host[i][j])) {
        std::cout << "Fail - The result is incorrect for element: [" << i << ", "
             << j << "], expected: " << c_host[i][j]
             << ", but found: " << c_back[i][j] << "\n";
        mismatch_found = true;
        print_count++;
        if (print_count == 5) break;
      }
    }

    if (print_count == 5) break;
  }

  if (!mismatch_found) {
    std::cout << "PASS\n";
  } else {
    std::cout << "FAIL\n";
  }
}
#endif
