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

// --- from kernels.cu ---
#include <math.h>

// double-precision atomic max
inline




// --- from lebesgue.h ---
double *chebyshev1 ( int n );
double *chebyshev2 ( int n );
double *chebyshev3 ( int n );
double *chebyshev4 ( int n );
double *equidistant1 ( int n );
double *equidistant2 ( int n );
double *equidistant3 ( int n );
double *fejer1 ( int n );
double *fejer2 ( int n );
double lebesgue_constant ( int n, double x[], int nfun, double xfun[] );
double lebesgue_function ( int n, double x[], int nfun, double xfun[] );
double *r8vec_linspace_new ( int n, double a, double b );
double r8vec_max ( int n, double r8vec[] );
void r8vec_print ( int n, double a[], const char *title );
void timestamp ( );
