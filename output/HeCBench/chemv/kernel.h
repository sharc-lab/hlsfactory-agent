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

// --- from kernel.cu ---



// --- from main.cu ---
/*
 * Copyright 2014      ARM Ltd.
 *
 * Use of this software is governed by the MIT license
 *
 * Compared to the original C example, the cpu and gpu results 
 * are compared for verification. -Zheming Jin
 *   
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>

#define REPEAT 1000
#define N 370
#define LDAT N
#define INCX 1
#define INCY 1
#define AT_SIZE (N * LDAT)
#define X_SIZE (N * INCX)
#define Y_SIZE (N * INCY)


/* chemv - complex hermitian matrix-vector multiplication
 * The function body was taken from a VOBLA-generated BLAS library.
 */




// --- from kernel.h ---
#ifndef KERNEL_H
#define KERNEL_H

struct ComplexFloat {
    float Re;
    float Im;
};

void chemv_kernel0(struct ComplexFloat *AT, struct ComplexFloat *X, struct ComplexFloat *Y, float alpha_im, float alpha_re, float beta_im, float beta_re);
void chemv_kernel1(struct ComplexFloat *AT, struct ComplexFloat *X, struct ComplexFloat *Y, float alpha_im, float alpha_re);

#define ppcg_min(x,y)    ({ __typeof__(x) _x = (x); __typeof__(y) _y = (y); _x < _y ? _x : _y; })
#define ppcg_max(x,y)    ({ __typeof__(x) _x = (x); __typeof__(y) _y = (y); _x > _y ? _x : _y; })

#endif
