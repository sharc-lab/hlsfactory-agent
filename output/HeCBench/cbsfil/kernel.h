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

// --- from main.cu ---
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>




// --- from kernels.h ---

#ifndef _CUBIC_BSPLINE_PREFILTER_KERNEL_H_
#define _CUBIC_BSPLINE_PREFILTER_KERNEL_H_

// The code below is based on the work of Philippe Thevenaz.
// See <http://bigwww.epfl.ch/thevenaz/interpolation/>

#define POLE (sqrtf(3.0f)-2.0f)  //pole for cubic b-spline

typedef unsigned int uint;
typedef unsigned char uchar;

float InitialCausalCoefficient(
    float* c,         // coefficients
    uint DataLength,  // number of coefficients
    int step)         // element interleave in bytes
{
  const uint Horizon = 12 < DataLength ? 12 : DataLength;

  // this initialization corresponds to clamping boundaries
  // accelerated loop
  float zn = POLE;
  float Sum = *c;
  for (uint n = 0; n < Horizon; n++) {
    Sum += zn * *c;
    zn *= POLE;
    c = (float*)((uchar*)c + step);
  }
  return(Sum);
}

float InitialAntiCausalCoefficient(
    float* c,         // last coefficient
    uint DataLength,  // number of samples or coefficients
    int step)         // element interleave in bytes
{
  // this initialization corresponds to clamping boundaries
  return((POLE / (POLE - 1.0f)) * *c);
}

void ConvertToInterpolationCoefficients(
    float* coeffs,    // input samples --> output coefficients
    uint DataLength,  // number of samples or coefficients
    int step)         // element interleave in bytes
{
  // compute the overall gain
  const float Lambda = (1.0f - POLE) * (1.0f - 1.0f / POLE);

  // causal initialization
  float* c = coeffs;
  float previous_c;  //cache the previously calculated c rather than look it up again (faster!)
  *c = previous_c = Lambda * InitialCausalCoefficient(c, DataLength, step);

  // causal recursion
  for (uint n = 1; n < DataLength; n++) {
    c = (float*)((uchar*)c + step);
    *c = previous_c = Lambda * *c + POLE * previous_c;
  }
  // anticausal initialization
  *c = previous_c = InitialAntiCausalCoefficient(c, DataLength, step);
  // anticausal recursion
  for (int n = DataLength - 2; 0 <= n; n--) {
    c = (float*)((uchar*)c - step);
    *c = previous_c = POLE * (previous_c - *c);
  }
}

void toCoef2DX(
    float* image,
    uint pitch,
    uint width,
    uint height)
{
  // process lines horizontally
  const uint y = _bid_x * BLOCK_DIM_X + _tid_x;
  if (y < height) {
    float* line = (float*)((uchar*)image + y * pitch);  //direct access
    ConvertToInterpolationCoefficients(line, width, sizeof(float));
  }
}

void toCoef2DY(
    float* image,
    uint pitch,
    uint width,
    uint height)
{
  // process lines vertically
  const uint x = _bid_x * BLOCK_DIM_X + _tid_x;
  if (x < width) {
    float* line = image + x;  //direct access
    ConvertToInterpolationCoefficients(line, height, pitch);
  }
}

#endif // _CUBIC_BSPLINE_PREFILTER_KERNEL_H_
