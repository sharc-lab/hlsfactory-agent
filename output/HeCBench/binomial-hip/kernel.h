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

// --- from kernel.cu ---
/*
 * Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
 *

// Overloaded shortcut functions for different precision modes
#ifndef DOUBLE_PRECISION
#else
#endif

// GPU kernel
#define THREADBLOCK_SIZE 128
#define ELEMS_PER_THREAD (NUM_STEPS/THREADBLOCK_SIZE)
#if NUM_STEPS % THREADBLOCK_SIZE
#error Bad constants
#endif


// Host-side interface to GPU binomialOptions
extern "C" void binomialOptionsGPU(
    real *callValue,
    TOptionData  *optionData,
    int optN,
    int numIterations
    )
{
  __TOptionData h_OptionData[MAX_OPTIONS];


  __TOptionData *d_OptionData;
  hipMalloc ((void**)&d_OptionData, sizeof(__TOptionData) * MAX_OPTIONS);
  hipMemcpy(d_OptionData, h_OptionData, optN * sizeof(__TOptionData), hipMemcpyHostToDevice);

  real *d_CallValue;
  hipMalloc ((void**)&d_CallValue, sizeof(real) * MAX_OPTIONS);

  hipDeviceSynchronize();
  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < numIterations; i++)
    hipLaunchKernelGGL(binomialOptionsKernel, dim3(optN), dim3(THREADBLOCK_SIZE), 0, 0, d_OptionData, d_CallValue);

  hipDeviceSynchronize();
  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average kernel execution time : %f (us)\n", time * 1e-3f / numIterations);

  hipMemcpy(callValue, d_CallValue, optN *sizeof(real), hipMemcpyDeviceToHost);
  hipFree(d_OptionData);
  hipFree(d_CallValue);
}


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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <chrono>


// Black-Scholes formula for binomial tree results validation
extern "C" void BlackScholesCall(
    real &callResult,
    TOptionData optionData
    );

// Process single option on CPU
// Note that CPU code is for correctness testing only and not for benchmarking.
extern "C" void binomialOptionsCPU(
    real &callResult,
    TOptionData optionData
    );

// Process an array of OptN options on GPU
extern "C" void binomialOptionsGPU(
    real *callValue,
    TOptionData  *optionData,
    int optN,
    int numIterations
    );

// Helper function, returning uniformly distributed
// random float in [low, high] range



// --- from reference.cu ---
/*
 * Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
 *

extern "C" void BlackScholesCall(
    real &callResult,
    TOptionData optionData
    )
{
  real S = optionData.S;
  real X = optionData.X;
  real T = optionData.T;
  real R = optionData.R;
  real V = optionData.V;

  real sqrtT = sqrt(T);
  real    d1 = (log(S / X) + (R + (real)0.5 * V * V) * T) / (V * sqrtT);
  real    d2 = d1 - V * sqrtT;
  real CNDD1 = CND(d1);
  real CNDD2 = CND(d2);

  //Calculate Call and Put simultaneously
  real expRT = exp(- R * T);
  callResult   = (real)(S * CNDD1 - X * expRT * CNDD2);
}

// Process an array of OptN options on CPU
// Note that CPU code is for correctness testing only and not for benchmarking.

extern "C" void binomialOptionsCPU(
    real &callResult,
    TOptionData optionData
    )
{
  static real Call[NUM_STEPS + 1];

  const real       S = optionData.S;
  const real       X = optionData.X;
  const real       T = optionData.T;
  const real       R = optionData.R;
  const real       V = optionData.V;

  const real      dt = T / (real)NUM_STEPS;
  const real     vDt = V * sqrt(dt);
  const real     rDt = R * dt;
  //Per-step interest and discount factors
  const real      If = exp(rDt);
  const real      Df = exp(-rDt);
  //Values and pseudoprobabilities of upward and downward moves
  const real       u = exp(vDt);
  const real       d = exp(-vDt);
  const real      pu = (If - d) / (u - d);
  const real      pd = 1.0 - pu;
  const real  puByDf = pu * Df;
  const real  pdByDf = pd * Df;

  // Compute values at expiration date:
  // call option value at period end is V(T) = S(T) - X
  // if S(T) is greater than X, or zero otherwise.
  // The computation is similar for put options.
  for (int i = 0; i <= NUM_STEPS; i++)
    Call[i] = expiryCallValue(S, X, vDt, i);

  // Walk backwards up binomial tree
  for (int i = NUM_STEPS; i > 0; i--)
    for (int j = 0; j <= i - 1; j++)
      Call[j] = puByDf * Call[j + 1] + pdByDf * Call[j];

  callResult = (real)Call[0];
}


// --- from binomialOptions.h ---
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

#ifndef BINOMIALOPTIONS_COMMON_H
#define BINOMIALOPTIONS_COMMON_H


////////////////////////////////////////////////////////////////////////////////
// Global types
////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    real S;
    real X;
    real T;
    real R;
    real V;
} TOptionData;

////////////////////////////////////////////////////////////////////////////////
// Global parameters
////////////////////////////////////////////////////////////////////////////////
//Number of time steps
#define   NUM_STEPS 2048
//Max option batch size
#define MAX_OPTIONS 1024

#define NUM_ITERATIONS 1000 

#endif


// --- from realtype.h ---
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

#ifndef REALTYPE_H
#define REALTYPE_H

//#define DOUBLE_PRECISION

#ifndef DOUBLE_PRECISION
typedef float real;
#else
typedef double real;
#endif

#endif
