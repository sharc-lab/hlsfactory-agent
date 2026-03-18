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

// --- from needle.cu ---
#define LIMIT -999
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

// includes, kernels

#ifdef TIMING
#include "timing.h"

struct timeval tv;
struct timeval tv_total_start, tv_total_end;
struct timeval tv_h2d_start, tv_h2d_end;
struct timeval tv_d2h_start, tv_d2h_end;
struct timeval tv_kernel_start, tv_kernel_end;
struct timeval tv_mem_alloc_start, tv_mem_alloc_end;
struct timeval tv_close_start, tv_close_end;
float init_time = 0, mem_alloc_time = 0, h2d_time = 0, kernel_time = 0,
      d2h_time = 0, close_time = 0, total_time = 0;
#endif

////////////////////////////////////////////////////////////////////////////////
// declaration, forward


////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////





// --- from needle_kernel.cu ---

#include <stdio.h>

#define SDATA( index)      CUT_BANK_CHECKER(sdata, index)






// --- from needle.h ---
#ifdef RD_WG_SIZE_0_0
	#define BLOCK_SIZE RD_WG_SIZE_0_0
#elif defined(RD_WG_SIZE_0)
	#define BLOCK_SIZE RD_WG_SIZE_0
#elif defined(RD_WG_SIZE)
	#define BLOCK_SIZE RD_WG_SIZE
#else
	#define BLOCK_SIZE 16
#endif
//#define TRACE

