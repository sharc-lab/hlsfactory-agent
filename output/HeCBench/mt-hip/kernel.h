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

// --- from MT.cu ---
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

///////////////////////////////////////////////////////////////////////////////
// This sample implements Mersenne Twister random number generator
// and Cartesian Box-Muller transformation on the GPU
///////////////////////////////////////////////////////////////////////////////

// standard utilities and systems includes
#include <stdio.h>
#include <hip/hip_runtime.h>

// comment the below line if not doing Box-Muller transformation
#define DO_BOXMULLER

// Reference CPU MT and Box-Muller transformation 
extern "C" void initMTRef(const char *fname);
extern "C" void RandomRef(float *h_Rand, int nPerRng, unsigned int seed);
#ifdef DO_BOXMULLER
extern "C" void BoxMullerRef(float *h_Rand, int nPerRng);
#endif

#include <chrono>
using namespace std::chrono;

///////////////////////////////////////////////////////////////////////////////
//Load twister configurations
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
// Main function 
///////////////////////////////////////////////////////////////////////////////


// --- from MT_gold.cu ---
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

#include <stdio.h>
#include <math.h>

static mt_struct MT[MT_RNG_COUNT];
static uint32_t state[MT_NN];

extern "C" void initMTRef(const char *fname){
    
    // open the file for binary read
    FILE* fd = fopen(fname, "rb");


    int iRng, iOut;

}


extern "C" void BoxMullerRef(float *h_Random, int NPerRng) {
    int i;

    for(i = 0; i < MT_RNG_COUNT * NPerRng; i += 2)
        BoxMuller(h_Random[i + 0], h_Random[i + 1]);
}


// --- from genmtrand.cu ---
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
 
 /* genmtrand.c */

/* Copyright (C) 2001-2006 Makoto Matsumoto and Takuji Nishimura.  */
/* This library is free software; you can redistribute it and/or   */
/* modify it under the terms of the GNU Library General Public     */
/* License as published by the Free Software Foundation; either    */
/* version 2 of the License, or (at your option) any later         */
/* version.                                                        */
/* This library is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.            */
/* See the GNU Library General Public License for more details.    */
/* You should have received a copy of the GNU Library General      */
/* Public License along with this library; if not, write to the    */
/* Free Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA   */ 
/* 02111-1307  USA                                                 */

/*
 * Victor Podlozhnyuk @ 05/13/2007:
 * 1) Fixed sgenrand_mt():
 *    - Fixed loop indexing: 'i' variable was off by one.
 *    - apply wmask right on the state element initialization instead
 *      of separate loop, which could produce machine-dependent results(wrong).
 * 2) Slightly reformatted sources to be included into CUDA SDK.
 */

#include <stdio.h>
#include <stdlib.h>

#define SHIFT1 18




// --- from MT.h ---
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

#ifndef MERSENNETWISTER_H
#define MERSENNETWISTER_H
#ifndef mersennetwister_h
#define mersennetwister_h

#define      DCMT_SEED 4172

typedef struct{
  unsigned int matrix_a;
  unsigned int mask_b;
  unsigned int mask_c;
  unsigned int seed;
} mt_struct_stripped;

#define   MT_RNG_COUNT 4096     // DO NOT CHANGE
#define          MT_MM 9
#define          MT_NN 19
#define       MT_WMASK 0xFFFFFFFFU
#define       MT_UMASK 0xFFFFFFFEU
#define       MT_LMASK 0x1U
#define      MT_SHIFT0 12
#define      MT_SHIFTB 7
#define      MT_SHIFTC 15
#define      MT_SHIFT1 18
#define PI 3.14159265358979f

#endif
#endif


// --- from dci.h ---
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
 
 /* dci.h */

/* Copyright (C) 2001-2006 Makoto Matsumoto and Takuji Nishimura.  */
/* This library is free software; you can redistribute it and/or   */
/* modify it under the terms of the GNU Library General Public     */
/* License as published by the Free Software Foundation; either    */
/* version 2 of the License, or (at your option) any later         */
/* version.                                                        */
/* This library is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.            */
/* See the GNU Library General Public License for more details.    */
/* You should have received a copy of the GNU Library General      */
/* Public License along with this library; if not, write to the    */
/* Free Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA   */ 
/* 02111-1307  USA                                                 */

/*
 * Victor Podlozhnyuk @ 05/13/2007:
 * Introduced typedef for uint32_t instead of C99-only #include<inttypes.h>
 */

#ifndef DC_IMPLEMENT
#define DC_IMPLEMENT

typedef unsigned int uint32_t;
#define UINT32_C(a) ((uint32_t)a)

typedef struct {
    uint32_t aaa;
    int mm,nn,rr,ww;
    uint32_t wmask,umask,lmask;
    int shift0, shift1, shiftB, shiftC;
    uint32_t maskB, maskC;
    int i;
    uint32_t *state;
}mt_struct;

////////////////////////////////////////////////////////////////////////////////
// Prototypes for original Matsumoto & Nishimura code
////////////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus)
extern "C" 
#endif
void sgenrand_mt(uint32_t seed, mt_struct *mts);
#if defined(__cplusplus)
extern "C" 
#endif
uint32_t genrand_mt(mt_struct *mts);

#endif
