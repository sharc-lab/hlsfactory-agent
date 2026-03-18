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

// --- from kernels.cu ---
#include <stdio.h>
#include <chrono>

// Thread block size
#define GROUP_SIZE 256


inline double4 operator+(double4 a, double4 b)
{
  return {s,s,s,s,s,s,s,s};
}

// Calculates equivalent distribution 

// convert_int8() may be language specific





// --- from main.cu ---
#include <stdio.h>
#include <stdlib.h>



// --- from utils.h ---
#ifndef UTILS_H
#define UTILS_H

#ifdef __HIPCC__
#include <hip/hip_runtime.h>
#else
#endif

typedef struct alignas(32)
{
  int s0, s1, s2, s3, s4, s5, s6, s7;
}
int8;

typedef struct alignas(64)
{
  double s0, s1, s2, s3, s4, s5, s6, s7;
}
double8;

void fluidSim (
  const int iterations,
  const double omega,
  const int *dims,
  const bool *h_type,
  double2 *u,
  double *rho,
  const double8 dirX,
  const double8 dirY,
  const double w[9],     // Weights
        double *h_if0,
        double *h_if1234,
        double *h_if5678,
        double *h_of0,
        double *h_of1234,
        double *h_of5678);

// Calculates equivalent distribution 
double computefEq(double rho, double weight, const double dir[2], const double velocity[2]);

#ifdef VERIFY
void reference (
  const int iterations,
  const double omega,
  const int *dims,
  const bool *h_type,
        double *rho,
  const double (*e)[2],  // Directions
  const double w[9],     // Weights
  const double *h_if0,
  const double *h_if1234,
  const double *h_if5678,
        double *v_of0,
        double *v_of1234,
        double *v_of5678);

void verify(
  const int *dims,
  const double *h_of0,
  const double *h_of1234,
  const double *h_of5678,
  const double *v_of0,
  const double *v_of1234,
  const double *v_of5678);
#endif

#endif
