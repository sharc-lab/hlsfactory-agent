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

// --- from distort.cu ---
#include <cmath>








// --- from main.cu ---
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>



// --- from distort.h ---
#ifndef DISTORTION_H
#define DISTORTION_H

#include <hip/hip_runtime.h>

struct Properties{
  float K;
  float centerX;
  float centerY;
  int width;
  int height;
  float thresh;
  float xscale;
  float yscale;
  float xshift;
  float yshift;
};

float calc_shift(float x1, float x2, float cx, float k, float thresh);

void reference (
  const uchar3* src,
        uchar3* dst,
  const struct Properties* prop);

void barrel_distort (
  const uchar3* src,
        uchar3* dst,
  const struct Properties* prop);

#endif
