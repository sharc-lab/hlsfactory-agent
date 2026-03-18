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

// --- from blackScholesAnalyticEngine.cu ---
//blackScholesAnalyticEngine.cu
//Scott Grauer-Gray
//Functions for running black scholes using the analytic engine (from Quantlib) on the GPU

#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
//needed for optionInputStruct
#include "blackScholesAnalyticEngineStructs.cuh"

//needed for the kernel(s) to run on the GPU

#define NUM_DIFF_SETTINGS 37

//function to run the black scholes analytic engine on the gpu



// --- from blackScholesAnalyticEngineKernels.cu ---
//blackScholesAnalyticEngineKernels.cu
//Scott Grauer-Gray
//Kernels for running black scholes using the analytic engine

#ifndef BLACK_SCHOLES_ANALYTIC_ENGINE_KERNELS_CU
#define BLACK_SCHOLES_ANALYTIC_ENGINE_KERNELS_CU

//declarations for the kernels
#include "blackScholesAnalyticEngineKernels.cuh"

//needed for the constants in the error function
#include "errorFunctConsts.cuh"

//device kernel to retrieve the compound factor in interestRate

//device kernel to retrieve the discount factor in interestRate

//device function to get the variance of the black volatility function

//device function to get the discount on a dividend yield

//device function to get the discount on the risk free rate

//device kernel to run the error function

//device kernel to run the operator function in cumulative normal distribution

//device kernel to run the gaussian function in the normal distribution

//device kernel to retrieve the derivative in a cumulative normal distribution

//device function to initialize the cumulative normal distribution structure

//device function to initialize variable in the black calculator

//device function to initialize the black calculator

//device function to retrieve the output resulting value

//global function to retrieve the output value for an option

#endif //BLACK_SCHOLES_ANALYTIC_ENGINE_KERNELS_CU



// --- from blackScholesAnalyticEngineKernelsCpu.cu ---
//blackScholesAnalyticEngineKernelsCpu.cu
//Scott Grauer-Gray
//Kernels for running black scholes using the analytic engine

#ifndef BLACK_SCHOLES_ANALYTIC_ENGINE_KERNELS_CPU_CU
#define BLACK_SCHOLES_ANALYTIC_ENGINE_KERNELS_CPU_CU

//declarations for the kernels
#include "blackScholesAnalyticEngineKernelsCpu.cuh"

//device kernel to retrieve the compound factor in interestRate

//device kernel to retrieve the discount factor in interestRate

//device function to get the variance of the black volatility function

//device function to get the discount on a dividend yield

//device function to get the discount on the risk free rate

//device kernel to run the error function

//device kernel to run the operator function in cumulative normal distribution

//device kernel to run the gaussian function in the normal distribution

//device kernel to retrieve the derivative in a cumulative normal distribution

//device function to initialize the cumulative normal distribution structure

//device function to initialize variable in the black calculator

//device function to initialize the black calculator

//device function to retrieve the output resulting value

//global function to retrieve the output value for an option

#endif //BLACK_SCHOLES_ANALYTIC_ENGINE_KERNELS_CPU_CU
