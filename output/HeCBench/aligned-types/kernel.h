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
 * Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

/*
 * This is a simple test showing performance differences
 * between aligned and misaligned structures
 * (those having/missing __align__ keyword).
 * It measures per-element copy throughput for
 * aligned and misaligned structures on
 * big chunks of data.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <chrono>

////////////////////////////////////////////////////////////////////////////////
// Misaligned types
////////////////////////////////////////////////////////////////////////////////
typedef unsigned char uchar_misaligned;

typedef unsigned short int ushort_misaligned;

typedef struct
{
  unsigned char r, g, b, a;
} uchar4_misaligned;

typedef struct
{
  unsigned int l, a;
} uint2_misaligned;

typedef struct
{
  unsigned int r, g, b;
} uint3_misaligned;

typedef struct
{
  unsigned int r, g, b, a;
} uint4_misaligned;

typedef struct
{
  uint4_misaligned c1, c2;
}
uint8_misaligned;

////////////////////////////////////////////////////////////////////////////////
// Aligned types
////////////////////////////////////////////////////////////////////////////////
uchar4_aligned;

typedef unsigned int uint_aligned;

uint2_aligned;

uint3_aligned;

uint4_aligned;

////////////////////////////////////////////////////////////////////////////////
// Because G80 class hardware natively supports global memory operations
// only with data elements of 4, 8 and 16 bytes, if structure size
// exceeds 16 bytes, it can't be efficiently read or written,
// since more than one global memory non-coalescable load/store instructions
// will be generated, even if __align__ option is supplied.
// "Structure of arrays" storage strategy offers best performance
// in general case. See section 5.1.2 of the Programming Guide.
////////////////////////////////////////////////////////////////////////////////
uint8_aligned;

////////////////////////////////////////////////////////////////////////////////
// Common host and device functions
////////////////////////////////////////////////////////////////////////////////
//Round a / b to nearest higher integer value

//Round a / b to nearest lower integer value

//Align a to nearest higher multiple of b

//Align a to nearest lower multiple of b

////////////////////////////////////////////////////////////////////////////////
// Copy is carried out on per-element basis,
// so it's not per-byte in case of padded structures.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Validation routine for simple copy kernel.
// We must know "packed" size of TData (number_of_fields * sizeof(simple_type))
// and compare only these "packed" parts of the structure,
// containing actual user data. The compiler behavior with padding bytes
// is undefined, since padding is merely a placeholder
// and doesn't contain any user data.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Data configuration
////////////////////////////////////////////////////////////////////////////////
//Memory chunk size in bytes. Reused for test
const int       MEM_SIZE = 50000000;
const int NUM_ITERATIONS = 1000;

//CPU input data and instance of GPU output data
unsigned char *h_idataCPU, *h_odataGPU;


