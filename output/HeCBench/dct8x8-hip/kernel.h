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

// --- from DCT8x8_gold.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *




// --- from kernels.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *







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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <hip/hip_runtime.h>

void DCT8x8(
    float* d_Dst,
    const float* d_Src,
    unsigned int stride,
    unsigned int imageH,
    unsigned int imageW,
    int dir
);


////////////////////////////////////////////////////////////////////////////////
// Main program
////////////////////////////////////////////////////////////////////////////////


// --- from DCT8x8.h ---
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

#ifndef DCT8x8_H
#define DCT8x8_H

////////////////////////////////////////////////////////////////////////////////
// Common definitions
////////////////////////////////////////////////////////////////////////////////
#define BLOCK_SIZE 8
#define BLOCK_X 32
#define BLOCK_Y 16
#define DCT_FORWARD 666
#define DCT_INVERSE 777

////////////////////////////////////////////////////////////////////////////////
// Hardcoded unrolled fast 8-point (i)DCT routines
////////////////////////////////////////////////////////////////////////////////
#define    C_a 1.3870398453221474618216191915664f  //a = sqrt(2) * cos(1 * pi / 16)
#define    C_b 1.3065629648763765278566431734272f  //b = sqrt(2) * cos(2 * pi / 16)
#define    C_c 1.1758756024193587169744671046113f  //c = sqrt(2) * cos(3 * pi / 16)
#define    C_d 0.78569495838710218127789736765722f //d = sqrt(2) * cos(5 * pi / 16)
#define    C_e 0.54119610014619698439972320536639f //e = sqrt(2) * cos(6 * pi / 16)
#define    C_f 0.27589937928294301233595756366937f //f = sqrt(2) * cos(7 * pi / 16)
#define C_norm 0.35355339059327376220042218105242f //1 / sqrt(8)

////////////////////////////////////////////////////////////////////////////////
// Reference CPU 8x8 (i)DCT
////////////////////////////////////////////////////////////////////////////////
void DCT8x8CPU(float *dst, const float *src, unsigned int stride, 
               unsigned int imageH, unsigned int imageW, int dir);

#endif
