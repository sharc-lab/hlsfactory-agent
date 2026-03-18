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

// --- from main.cu ---
//-----------------------------------------------------------------------
// Reference
//
// Harris, M. and Garland, M., 2012.
// Optimizing parallel prefix operations for the Fermi architecture.
// In GPU Computing Gems Jade Edition (pp. 29-38). Morgan Kaufmann.
//-----------------------------------------------------------------------

#include <cstdio>
#include <cstring>
#include <chrono>



// warp scan optimized for binary

// positive numbers
__inline__



template <int N>

