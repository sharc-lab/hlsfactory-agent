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
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <hip/hip_runtime.h>
#include <hip/hip_fp16.h>
#include <hipblas/hipblas.h>

#define TILE_X 16
#define TILE_Y 16

// M * K, K * N
template <typename T>

template <typename T>

//
// Main example for Gemm consisting of
// initialization of A, B and C matrices as well as
// scalars alpha and beta.  Then the product
//
// C = alpha * op(A) * op(B) + beta * C
//
// is performed and finally the results are post processed.
//
template <typename fp>

//
// Main entry point for example.
//


// --- from utils.h ---
#ifdef DEBUG
template <typename T>
void print_2x2_matrix_values(T M, int ldM, std::string M_name)
{
  std::cout << std::endl;
  std::cout << "\t\t\t" << M_name << " = [ " << (float)M[0*ldM + 0] << ", " << (float)M[1*ldM + 0]         << ", ...\n";
  std::cout << "\t\t\t    [ "                << (float)M[0*ldM + 1] << ", " << (float)M[1*ldM + 1] << ", ...\n";
  std::cout << "\t\t\t    [ "                << "...\n";
  std::cout << std::endl;
}
#endif

//
// helpers for initializing templated scalar data type values.
//
template <typename fp> void rand_matrix(fp *M, int n_row, int n_col)
{
  for (int i = 0; i < n_row; i++)
    for (int j = 0; j < n_col; j++)
      M[i * n_col + j] = rand() % 2;
}

void performance (int m, int n, int k, bool is_integer, double avg_time) {
  double total_ops = double(m) * double(n) * double(k) * 2 + 
                     double(m) * double(n);
  std::cout << "Average GEMM execution time: " << avg_time * 1e-3 <<  " (us), ";
  double perf = total_ops / avg_time;

  auto scale_string = "G";
  auto unit_string = is_integer ? "OP/s" : "FLOP/s";

  if (perf >= 1000) {
    perf /= 1000;
    scale_string = "T";
  }

  std::cout << perf << " " << scale_string << unit_string << std::endl;
}
