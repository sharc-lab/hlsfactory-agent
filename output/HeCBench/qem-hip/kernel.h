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

// --- from gpu_solver.cu ---
#include <algorithm>
#include <stdio.h>
#include <hip/hip_runtime.h>

// Convenience function for checking CUDA runtime API results
// can be wrapped around any runtime API call. No-op in release builds.



// --- from main.cu ---
#include <algorithm>
#include <chrono> // for high_resolution_clock
#include <cstdio>
#include <random>

#include "reference.h"




// --- from gpu_solver.h ---
#ifndef GPU_SOLVER_CUH
#define GPU_SOLVER_CUH

#include <hip/hip_runtime.h>

hipError_t checkHip(hipError_t result);

void cubicSolver(int n, float *A, float *B, float *C, float *D,
                            float *Q, float *R, float *del, float *theta,
                            float *sqrtQ, float *x1, float *x2, float *x3,
                            float *x1_img, float *x2_img, float *x3_img);

void QRdel(int n, float *A, float *B, float *C, float *D, float *b,
                      float *c, float *d, float *Q, float *R, float *Qint,
                      float *Rint, float *del);

void QuarticSolver(int n, float *A, float *B, float *C, float *D,
                              float *b, float *Q, float *R, float *del,
                              float *theta, float *sqrtQ, float *x1, float *x2,
                              float *x3, float *temp, float *min);

void QuarticSolver_full(int n, float *A, float *B, float *C,
                                   float *D, float *b, float *c, float *d,
                                   float *Q, float *R, float *del, float *theta,
                                   float *sqrtQ, float *x1, float *x2,
                                   float *x3, float *temp, float *min);

void QuarticMinimumGPU(int N, float *A, float *B, float *C, float *D, float *E,
                       float *min);

void QuarticMinimumGPUStreams(int N, float *A, float *B, float *C, float *D,
                              float *E, float *min);

#endif
