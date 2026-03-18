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
//**********************************************************//
//   Matching test code by Marten Bjorkman aka Celebrandil  //
//                                                          //
//   The code includes an example of gradual optimization   //
//   of a kernel for matching two sets of 16K 128D points.  //
//**********************************************************//

#include <cstring>
#include <cmath>
#include <iostream>
#include <vector>
#include <memory>  // std::align
#include <chrono>

#define NPTS (2048*8)
#define NDIM 128

#define M1W  128
#define M2W   16
#define M2H   16
#define M5W   16
#define M5H   16
#define M5R    4
#define M7W   32
#define M7H   32
#define M7R    4

// serial execution on a host

// verify host and device results
      











