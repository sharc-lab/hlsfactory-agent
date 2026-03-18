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

// --- from HiddenMarkovModel.cu ---
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

#include <stdlib.h>
#include <stdio.h>

// forward declaractions
int initHMM(float *initProb, float *mtState, float *mtObs, const int &nState, const int &nEmit);
int ViterbiCPU(float &viterbiProb,
               int *viterbiPath,
               int *obs, 
               const int &nObs, 
               float *initProb,
               float *mtState, 
               const int &nState,
               float *mtEmit);
int ViterbiGPU(float &viterbiProb,
               int * viterbiPath,
               int * obs, 
               const int nObs, 
               float * initProb,
               float * mtState, 
               const int nState,
               const int nEmit,
               float * mtEmit);

// main function
//*****************************************************************************

// initialize initial probability, state transition matrix and emission matrix with random 
// numbers. Note that this does not satisfy the normalization property of the state matrix. 
// However, since the algorithm does not use this property, for testing purpose this is fine.
//*****************************************************************************


// --- from ViterbiCPU.cu ---
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
 
#include <cstdlib>
#include <cstdio>

///////////////////////////////////////////////////////////////////////////////
// Using Viterbi algorithm to search for a Hidden Markov Model for the most
// probable state path given the observation sequence.
///////////////////////////////////////////////////////////////////////////////


// --- from ViterbiGPU.cu ---
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
#include <cstdlib>
#include <cstdio>


// Using Viterbi algorithm to search for a Hidden Markov Model for the most
// probable state path given the observation sequence.
