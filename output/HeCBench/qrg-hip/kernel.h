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

// --- from main.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

///////////////////////////////////////////////////////////////////////////////
// This sample implements Niederreiter quasirandom number generator
// and Moro's Inverse Cumulative Normal Distribution generator
///////////////////////////////////////////////////////////////////////////////

// standard utilities and systems includes
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <hip/hip_runtime.h>

// forward declarations
void initQuasirandomGenerator(unsigned int *table);
double getQuasirandomValue63(INT64 i, int dim);

////////////////////////////////////////////////////////////////////////////////
// Moro's Inverse Cumulative Normal Distribution function approximation
////////////////////////////////////////////////////////////////////////////////

// size of output random array
const unsigned int N = 1048576;





// --- from reference.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

// standard utilities and systems includes
#include <math.h>

////////////////////////////////////////////////////////////////////////////////
// Table generation functions
////////////////////////////////////////////////////////////////////////////////
// Internal 64(63)-bit table
static INT64 cjn[63][QRNG_DIMENSIONS];


////////////////////////////////////////////////////////////////////////////////
//  @misc{Bratley92:LDS,
//    author = "B. Fox and P. Bratley and H. Niederreiter",
//    title = "Implementation and test of low discrepancy sequences",
//    text = "B. L. Fox, P. Bratley, and H. Niederreiter. Implementation and test of
//      low discrepancy sequences. ACM Trans. Model. Comput. Simul., 2(3):195--213,
//      July 1992.",
//    year = "1992" }
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Initialization (table setup)
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//Generate 63-bit quasirandom number for given index and dimension and normalize
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Moro's Inverse Cumulative Normal Distribution function approximation
////////////////////////////////////////////////////////////////////////////////


// --- from qrg.h ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#ifndef QRG_COMMON_H
#define QRG_COMMON_H

////////////////////////////////////////////////////////////////////////////////
// Global types and constants
////////////////////////////////////////////////////////////////////////////////
typedef long long int INT64;

#define QRNG_DIMENSIONS 3
#define QRNG_RESOLUTION 31
#define INT_SCALE (1.0f / (float)0x80000001U)

#endif
