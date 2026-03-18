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
// Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
//
// Please refer to the NVIDIA end user license agreement (EULA) associated
// with this source code for terms and conditions that govern your use of
// this software. Any use, reproduction, disclosure, or distribution of
// this software and related documentation outside the terms of the EULA
// is strictly prohibited.

// ----------------------------------------------------------------------------------------
// Transpose
//
// This file contains both device and host code for transposing a floating-point
// matrix.  It performs several transpose kernels, which incrementally improve performance
// through coalescing, removing shared memory bank conflicts, and eliminating partition
// camping.  Several of the kernels perform a copy, used to represent the best case
// performance that a transpose can achieve.
// ----------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <chrono>

#define checkCudaErrors(call)                                                           \
  do {                                                                                  \
    cudaError_t err = call;                                                             \
  } while (0)

// Utilities and system includes

// Each block transposes/copies a tile of TILE_DIM x TILE_DIM elements
// using TILE_DIM x BLOCK_ROWS threads, so that each thread transposes
// TILE_DIM/BLOCK_ROWS elements.  TILE_DIM must be an integral multiple of BLOCK_ROWS

#define TILE_DIM    16
#define BLOCK_ROWS  16

// This sample assumes that MATRIX_SIZE_X = MATRIX_SIZE_Y
int MATRIX_SIZE_X = 1024;
int MATRIX_SIZE_Y = 1024;
int MUL_FACTOR    = TILE_DIM;

#define FLOOR(a,b) (a-(a%b))

// Compute the tile size necessary to illustrate performance cases for SM20+ hardware
int MAX_TILES = (FLOOR(MATRIX_SIZE_X,512) * FLOOR(MATRIX_SIZE_Y,512)) / (TILE_DIM *TILE_DIM);

// -------------------------------------------------------
// Copies
// width and height must be integral multiples of TILE_DIM
// -------------------------------------------------------



// -------------------------------------------------------
// Transposes
// width and height must be integral multiples of TILE_DIM
// -------------------------------------------------------


// coalesced transpose (with bank conflicts)


// Coalesced transpose with no bank conflicts


// Transpose that effectively reorders execution of thread blocks along diagonals of the
// matrix (also coalesced and has no bank conflicts)
//
// Here _bid_x is interpreted as the distance along a diagonal and _bid_y as
// corresponding to different diagonals
//
// blockIdx_x and blockIdx_y expressions map the diagonal coordinates to the more commonly
// used cartesian coordinates so that the only changes to the code from the coalesced version
// are the calculation of the blockIdx_x and blockIdx_y and replacement of _bid_x and
// bloclIdx.y with the subscripted versions in the remaining code


// --------------------------------------------------------------------
// Partial transposes
// NB: the coarse- and fine-grained routines only perform part of a
//     transpose and will fail the test against the reference solution
//
//     They are used to assess performance characteristics of different
//     components of a full transpose
// --------------------------------------------------------------------



// ---------------------
// host utility routines
// ---------------------



