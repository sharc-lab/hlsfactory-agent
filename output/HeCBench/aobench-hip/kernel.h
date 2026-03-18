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

// --- from ao.cu ---
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <chrono>
#include <hip/hip_runtime.h>

#define WIDTH        256
#define HEIGHT       256
#define NSUBSAMPLES  2
#define NAO_SAMPLES  8
#define BLOCK_SIZE   16

typedef struct _vec
{
  float x;
  float y;
  float z;
} vec;

typedef struct _Isect
{
  float t;
  vec    p;
  vec    n;
  int    hit; 
} Isect;

typedef struct _Sphere
{
  vec    center;
  float radius;

} Sphere;

typedef struct _Plane
{
  vec    p;
  vec    n;

} Plane;

typedef struct _Ray
{
  vec    org;
  vec    dir;
} Ray;







class RNG {
  public:
    unsigned int x;
    const int fmask = (1 << 23) - 1;   
};






#define gpuErrchk(ans) gpuAssert((ans), __FILE__, __LINE__)



