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
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <hip/hip_runtime.h>

// Color encoding of flow vectors
// adapted from the color circle idea described at
//   http://members.shaw.ca/quadibloc/other/colint.htm
//
// Daniel Scharstein, 4/2007

#define RY  15
#define YG  6
#define GC  4
#define CB  11
#define BM  13
#define MR  6
#define MAXCOLS  (RY + YG + GC + CB + BM + MR)
typedef unsigned char uchar;




