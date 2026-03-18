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
/***************************************************************************
 *
 *            (C) Copyright 2007 The Board of Trustees of the
 *                        University of Illinois
 *                         All Rights Reserved
 *
 ***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

/*
#define CUDA_ERRCK \

        float mark;
} Place;

typedef struct {
        int from1, from2;
        int to1, to2;
} Transition;

// this starts from row 0 and col 0
P(r,c)    -> T(r,c)   -> P(r,c+1)  ->
  |            |            |
 \/           \/           \/
T(r+1,c-1)-> P(r+1,c) -> T(r+1,c)  ->
  |            |            |
 \/           \/           \/
P(r+2,c)  -> T(r+2,c) -> P(r+2,c+1)->
  |            |            |
 \/           \/           \/
T(r+3,c-1)-> P(r+3,c) -> T(r+3,c)->
  |            |            |
 \/           \/           \/

*/

static int N, S, T, NSQUARE2;
uint32 host_mt[MERS_N];

void* AllocateDeviceMemory(int size);
void CopyFromDeviceMemory(void* h_p, void* d_p, int size);
void CopyFromHostMemory(void* d_p, void* h_p, int size);
void FreeDeviceMemory(void* mem);
void PetrinetOnDevice(long long &ktime);
void compute_statistics();

float results[4];
float* h_vars;
int* h_maxs;





// Allocate a device matrix of same size as M.

// Copy device memory to host memory

// Copy device memory from host memory

// Free a device matrix.


// --- from petri_kernel.cu ---
/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/
#ifndef _PETRINET_KERNEL_H_
#define _PETRINET_KERNEL_H_

#include <stdio.h>

#define BLOCK_SIZE 256
#define BLOCK_SIZE_BITS 8





// Kernel function for simulating Petri Net for a defined grid
// n: the grid has 2nX2n places and transitions together
// s: steps in each trajectory
// t: number of trajectories

#endif // #ifndef _PETRINET_KERNEL_H_


// --- from rand_gen.cu ---
/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

#define LOWER_MASK ((1LU << MERS_R) - 1)         
#define UPPER_MASK (0xFFFFFFFF << MERS_R)        

/* 
   The following two functions implement the Mersenne Twister random 
   number generator.  The copyright notice/disclaimer, etc are related
   to this code.

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


// return a random in [0, max]
// #define Random(max_plus_1)  (BRandom()% (max_plus_1))



// --- from petri.h ---
/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/
#ifndef _PETRI_H_
#define _PETRI_H_

#define MAX_DEVICE_MEM 750000000

void PetrinetKernel(int* g_places, int n, int s, int t);

#endif // _PETRI_H_



// --- from randomc.h ---
/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/
#ifndef RANDOMC_H
#define RANDOMC_H

#include <math.h>          // default math function linrary

#include <assert.h>
#include <stdio.h>

// Define 32 bit signed and unsigned integers.
// Change these definitions, if necessary, on 64 bit computers
typedef   signed int int32; 
typedef unsigned int uint32; 

// constants for MT19937:
#define MERS_N   624
#define MERS_M   397
#define MERS_R   31
#define MERS_U   11
#define MERS_S   7
#define MERS_T   15
#define MERS_L   18
#define MERS_A   0x9908B0DF
#define MERS_B   0x9D2C5680
#define MERS_C   0xEFC60000

void RandomInit(uint32 seed); // re-seed
void BRandom();               // output random bits
uint32 mt[MERS_N]; // state vector

#endif

