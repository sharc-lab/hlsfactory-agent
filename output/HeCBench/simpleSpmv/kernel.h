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

// --- from kernels.cu ---
#include <stdlib.h>
#include <chrono>

// sparse matrix vector multiply using the CSR format

// vector sparse matrix vector multiply using the CSR format
template <int BS>

// dense matrix vector multiply



// Reference
// https://github.com/ROCm/rocm-blogs/blob/release/blogs/high-performance-computing/spmv/part-1/examples/vector_csr.cpp



// --- from mv.h ---
#ifndef SPMV_H
#define SPMV_H

#define REAL float
//#define REAL double
void mv_csr_serial( const size_t num_rows, const size_t *row_indices, const size_t *col_indices, const REAL *data, const REAL *x, REAL *y);
long mv_csr_parallel(const int repeat, const int bs, const size_t num_rows, const size_t *row_indices, const size_t *col_indices, const REAL *data, const REAL *x, const size_t nnz, REAL *matrix, REAL *y);
long vector_mv_csr_parallel(const int repeat, const int bs, const size_t num_rows, const size_t *row_indices, const size_t *col_indices, const REAL *data, const REAL *x, const size_t nnz, REAL *matrix, REAL *y);
long mv_dense_parallel(const int repeat, const int bs, const size_t num_rows, const REAL *x, REAL* matrix, REAL *y);
float check(REAL *A, REAL *B, size_t n);
void init_vector(REAL *vector, size_t m);
void init_matrix(REAL *matrix, size_t num_rows, size_t nnz);
void init_csr(size_t *row_indices, REAL *values, size_t *col_indices, REAL *matrix, size_t num_rows, size_t nnz);

#endif
