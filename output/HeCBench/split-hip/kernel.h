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
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *

//----------------------------------------------------------------------------
// scan4 scans 4*RadixSort::CTA_SIZE numElements in a block (4 per thread), using 
// a warp-scan algorithm
//----------------------------------------------------------------------------



  


// --- from verify.cu ---
#include <string.h>


