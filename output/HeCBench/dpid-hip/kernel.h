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

// --- from kernels.cu ---
// Copyright (c) 2016 Nicolas Weber and Sandra C. Amend / GCC / TU-Darmstadt. All rights reserved. 
// Use of this source code is governed by the BSD 3-Clause license that can be
// found in the LICENSE file.
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <chrono>

#define THREADS 128
#define WSIZE 32
#define TSIZE (THREADS / WSIZE)

#define TX _tid_x
#define PX (_bid_x * TSIZE + (TX / WSIZE))
#define PY _bid_y
#define WTHREAD  (TX % WSIZE)

inline

inline

inline

inline
void operator+=(float4& output, const float4 value) {
  output.x += value.x;
  output.y += value.y;
  output.z += value.z;
  output.w += value.w;
}

struct Local {
  float sx, ex, sy, ey;
  uint32_t sxr, syr, exr, eyr, xCount, yCount, pixelCount;

};

inline

// https://devblogs.nvidia.com/parallelforall/faster-parallel-reductions-kepler/
inline

inline

inline


inline




// --- from main.cu ---
// Copyright (c) 2016 Nicolas Weber and Sandra C. Amend / GCC / TU-Darmstadt.
// All rights reserved. 
// Use of this source code is governed by the BSD 3-Clause license that can be
// found in the LICENSE file.

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstdint>




// --- from shared.h ---
#ifndef _SHARED
#define _SHARED

#include <hip/hip_runtime.h>

struct Params {
  uint32_t oWidth;
  uint32_t oHeight;
  uint32_t iWidth;
  uint32_t iHeight;
     float pWidth;
     float pHeight;
     float lambda;
  uint32_t repeat;
};

#endif
