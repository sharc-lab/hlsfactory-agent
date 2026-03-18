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

// --- from murmurhash3.cu ---
//-------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
//-------------------------------------------------------------------------

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <chrono>

#define BLOCK_SIZE 256

#define FORCE_INLINE inline __attribute__((always_inline))


#define BIG_CONSTANT(x) (x##LU)

// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

// Finalization mix - force all bits of a hash block to avalanche



