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

// --- from kernels.cu ---
/**************************************************************************
 **
 **  svd3
 **
 **  Quick singular value decomposition as described by:
 **  A. McAdams, A. Selle, R. Tamstorf, J. Teran and E. Sifakis,
 **  Computing the Singular Value Decomposition of 3x3 matrices
 **  with minimal branching and elementary floating point operations,
 **  University of Wisconsin - Madison technical report TR1690, May 2011
 **
 **  Identical GPU version
 **   Implementated by: Kui Wu
 **  kwu@cs.utah.edu
 **
 **  May 2018
 **
 **************************************************************************/

#define gone          1065353216
#define gsine_pi_over_eight    1053028117
#define gcosine_pi_over_eight   1064076127
#define gone_half        0.5f
#define gsmall_number      1.e-12f
#define gtiny_number      1.e-20f
#define gfour_gamma_squared    5.8284273147583007813f

#define __fadd_rn(a,b) ((a)+(b))
#define __fsub_rn(a,b) ((a)-(b))
#define __frsqrt_rn(a) (1.f / sqrtf(a))

union un { float f; unsigned int ui; };

inline



// --- from main.cu ---
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <cstdlib>
#include <chrono>



