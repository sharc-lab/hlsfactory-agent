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

// --- from RadixSort.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *



//----------------------------------------------------------------------------
// Perform one step of the radix sort.  Sorts by "nbits" key bits per step, 
// starting at startbit.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Main key-only radix sort function.  Sorts in place in the keys and values 
// arrays, but uses the other device arrays as temporary storage.  All pointer 
// parameters are device pointers.  Uses cudppScan() for the prefix sum of
// radix counters.
//----------------------------------------------------------------------------


// --- from RadixSort_kernels.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *

//----------------------------------------------------------------------------
// scan4 scans 4*RadixSort::CTA_SIZE numElements in a block (4 per thread), using 
// a warp-scan algorithm
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// Given an array with blocks sorted according to a 4-bit radix group, each 
// block counts the number of keys that fall into each radix in the group, and 
// finds the starting offset of each radix in the block.  It then writes the radix 
// counts to the counters array, and the starting offsets to the blockOffsets array.
//
// Template parameters are used to generate efficient code for various special cases
// For example, we have to handle arrays that are a multiple of the block size 
// (fullBlocks) differently than arrays that are not. "loop" is used when persistent 
// CTAs are used. 
//
// By persistent CTAs we mean that we launch only as many thread blocks as can 
// be resident in the GPU and no more, rather than launching as many threads as
// we have elements. Persistent CTAs loop over blocks of elements until all work
// is complete.  This can be faster in some cases.  In our tests it is faster
// for large sorts (and the threshold is higher on compute version 1.1 and earlier
// GPUs than it is on compute version 1.2 GPUs.
//                                
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// reorderData shuffles data in the array globally after the radix offsets 
// have been found. On compute version 1.1 and earlier GPUs, this code depends 
// on RadixSort::CTA_SIZE being 16 * number of radices (i.e. 16 * 2^nbits).
// 
// On compute version 1.1 GPUs ("manualCoalesce=true") this function ensures
// that all writes are coalesced using extra work in the kernel.  On later
// GPUs coalescing rules have been relaxed, so this extra overhead hurts 
// performance.  On these GPUs we set manualCoalesce=false and directly store
// the results.
//
// Template parameters are used to generate efficient code for various special cases
// For example, we have to handle arrays that are a multiple of the block size 
// (fullBlocks) differently than arrays that are not.  "loop" is used when persistent 
// CTAs are used. 
//
// By persistent CTAs we mean that we launch only as many thread blocks as can 
// be resident in the GPU and no more, rather than launching as many threads as
// we have elements. Persistent CTAs loop over blocks of elements until all work
// is complete.  This can be faster in some cases.  In our tests it is faster
// for large sorts (and the threshold is higher on compute version 1.1 and earlier
// GPUs than it is on compute version 1.2 GPUs.
//----------------------------------------------------------------------------


// --- from Scan.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *



// main exclusive scan routine


// --- from Scan_kernels.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *

////////////////////////////////////////////////////////////////////////////////
// Scan codelets
////////////////////////////////////////////////////////////////////////////////
#if(1)
//Naive inclusive scan: O(N * log2(N)) operations
//Allocate 2 * 'size' local memory, initialize the first half
//with 'size' zeros avoiding if(pos >= offset) condition evaluation
//and saving instructions


#else
#define LOG2_WARP_SIZE 5U
#define      WARP_SIZE (1U << LOG2_WARP_SIZE)

//Almost the same as naiveScan1 but doesn't need barriers
//assuming size <= WARP_SIZE



#endif

//Vector scan: the array to be scanned is stored
//in work-item private memory as uint4


////////////////////////////////////////////////////////////////////////////////
// Scan kernels
////////////////////////////////////////////////////////////////////////////////

//Exclusive scan of top elements of bottom-level scans (4 * THREADBLOCK_SIZE)

//Final step of large-array scan: combine basic inclusive scan with exclusive scan of top elements of input arrays


// --- from main.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#include <chrono>

void makeRandomUintVector(unsigned int *a, unsigned int numElements, unsigned int keybits);
bool verifySortUint(unsigned int *keysSorted, 
    unsigned int *valuesSorted, 
    unsigned int *keysUnsorted, 
    unsigned int len);



// assumes the values were initially indices into the array, for simplicity of 
// checking correct order of values


// --- from RadixSort.h ---
/*
* Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
*
* Please refer to the NVIDIA end user license agreement (EULA) associated
* with this source code for terms and conditions that govern your use of
* this software. Any use, reproduction, disclosure, or distribution of
* this software and related documentation outside the terms of the EULA
* is strictly prohibited.
*
*/
#ifndef _RADIXSORT_H_
#define _RADIXSORT_H_

#include <stdio.h>
#include <stdlib.h>
#include <hip/hip_runtime.h>

static const unsigned int WARP_SIZE = 32;
static const unsigned int bitStep = 4;
static const unsigned int CTA_SIZE = 128;

void radixSortKeys(unsigned int* d_keys, 
                   unsigned int* d_tempKeys, 
                   unsigned int* d_counters, 
                   unsigned int* d_blockOffsets, 
                   unsigned int* d_countersSum, 
                   unsigned int* d_buffer, 
                   const unsigned int numElements, 
                   const unsigned int keyBits, 
                   const unsigned int batchSize 
);

#endif


// --- from Scan.h ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */
#ifndef _SCAN_H_
#define _SCAN_H_

#include <assert.h>
#include <stdio.h>
#include <hip/hip_runtime.h>

#define MAX_WORKGROUP_INCLUSIVE_SCAN_SIZE 1024
#define MAX_LOCAL_GROUP_SIZE 256
static const int WORKGROUP_SIZE = 256;
static const unsigned int   MAX_BATCH_ELEMENTS = 64 * 1048576;
static const unsigned int MIN_SHORT_ARRAY_SIZE = 4;
static const unsigned int MAX_SHORT_ARRAY_SIZE = 4 * WORKGROUP_SIZE;
static const unsigned int MIN_LARGE_ARRAY_SIZE = 8 * WORKGROUP_SIZE;
static const unsigned int MAX_LARGE_ARRAY_SIZE = 4 * WORKGROUP_SIZE * WORKGROUP_SIZE;

unsigned int factorRadix2(unsigned int& log2L, unsigned int L);

void scanExclusiveLarge(
    unsigned int* d_Dst,
    unsigned int* d_Src,
    unsigned int* d_Buf,
    const unsigned int batchSize,
    const unsigned int arrayLength,
    const unsigned int numElements);

inline uint4 make_uint4(uint s)
{
    return make_uint4(s, s, s, s);
}

#endif
