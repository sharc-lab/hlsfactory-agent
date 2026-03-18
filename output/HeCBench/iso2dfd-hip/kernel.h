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

// --- from iso2dfd.cu ---
//==============================================================
// Copyright © 2019 Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

// ISO2DFD: the 2D-Finite-Difference-Wave Propagation, 
//
// ISO2DFD is a finite difference stencil kernel for solving the 2D acoustic
// isotropic wave equation. Kernels in this sample are implemented as 2nd order
// in space, 2nd order in time scheme without boundary conditions.
// The sample will explicitly run on the GPU as well as CPU to
// calculate a result.  If successful, the output will include GPU device name.
//
// this code sample can be found at :
// https://software.intel.com/en-us/articles/code-sample-two-dimensional-finite-difference-wave-propagation-in-isotropic-media-iso2dfd

#include <fstream>
#include <iostream>
#include <hip/hip_runtime.h>

#define BLOCK_SIZE 16
#define MIN(a, b) (a) < (b) ? (a) : (b)

/*
 * Host-Code
 * Utility function to display input arguments
 */

/*
 * Host-Code
 * Function used for initialization
 */

/*
 * Host-Code
 * Utility function to calculate L2-norm between resulting buffer and reference
 * buffer
 */

/*
 * Host-Code
 * CPU implementation for wavefield modeling
 * Updates wavefield for the number of iterations given in nIteratons parameter
 */

/*
 * Device-Code - GPU
 * Range kernel is used to spawn work-items in x, y dimension
 *
 */



// --- from iso2dfd.h ---
//==============================================================
// Copyright © 2020 Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

#include <chrono>
#include <cmath>
#include <cstring>

/*
 * Parameters to define coefficients
 * HALF_LENGTH: Radius of the stencil
 * Sample source code is tested for HALF_LENGTH=1 resulting in
 * 2nd order Stencil finite difference kernel
 */
#define DT 0.002f
#define DXY 20.0f
#define HALF_LENGTH 1

void usage(std::string);
