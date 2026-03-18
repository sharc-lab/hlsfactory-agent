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

// --- from computeQ.cu ---
/***************************************************************************
 *
 *            (C) Copyright 2007 The Board of Trustees of the
 *                        University of Illinois
 *                         All Rights Reserved
 *
 ***************************************************************************/

#define PI   3.1415926535897932384626433832795029f
#define PIx2 6.2831853071795864769252867665590058f

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define K_ELEMS_PER_GRID 2048

#define KERNEL_PHI_MAG_THREADS_PER_BLOCK 256
#define KERNEL_Q_THREADS_PER_BLOCK 256
#define KERNEL_Q_K_ELEMS_PER_GRID 1024

#define HIP_ERRCK							\
  {hipError_t err;							\
  }

struct kValues {
  float Kx;
  float Ky;
  float Kz;
  float PhiMag;
};

/* Values in the k-space coordinate system are stored in constant memory
 * on the GPU */
kValues ck[KERNEL_Q_K_ELEMS_PER_GRID];








// --- from main.cu ---
/***************************************************************************
 *
 *            (C) Copyright 2007 The Board of Trustees of the
 *                        University of Illinois
 *                         All Rights Reserved
 *
 ***************************************************************************/

/* 
 * C code for creating the Q data structure for fast convolution-based 
 * Hessian multiplication for arbitrary k-space trajectories.
 *
 * Inputs:
 * kx - VECTOR of kx values, same length as ky and kz
 * ky - VECTOR of ky values, same length as kx and kz
 * kz - VECTOR of kz values, same length as kx and ky
 * x  - VECTOR of x values, same length as y and z
 * y  - VECTOR of y values, same length as x and z
 * z  - VECTOR of z values, same length as x and y
 * phi - VECTOR of the Fourier transform of the spatial basis 
 *      function, evaluated at [kx, ky, kz].  Same length as kx, ky, and kz.
 *
 * recommended g++ options:
 *  -O3 -lm -ffast-math -funroll-all-loops
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <malloc.h>
#include <chrono>
#include <hip/hip_runtime.h>
#include "file.h"



