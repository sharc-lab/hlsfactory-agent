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
/*---------------------------------------------------------------
  Original author: Zebulun Arendsee
  March 26, 2013
----------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <hip/hip_runtime.h>
#include <hiprand/hiprand_kernel.h>

#define CUDA_CALL(x) do { if((x) != hipSuccess) { \
  printf("Error at %s:%d\n",__FILE__,__LINE__); \
  return EXIT_FAILURE;}} while(0)

#define PI 3.14159265359f
#define THREADS_PER_BLOCK 256
#define THREADS_PER_BLOCK_ADD 256

/* 
   Box-Muller Transformation: Generate one standard normal variable.

   This algorithm can be more efficiently used by producing two
   random normal variables. However, for the CPU, much faster

/*
   Generate random gamma variables on a CPU.
 */

/*
   Metropolis algorithm for producing random a values. 
   The proposal distribution in normal with a variance that
   is adjusted at each step.
 */

/*

/* 
   Generate a single Gamma distributed random variable by the Marsoglia 

/* 
   Initializes GPU random number generators 
 */

/*
   Sample each theta from the appropriate gamma distribution
 */

/* 
   Sampling of a and b require the sum and product of all theta 
   values. This function performs parallel summations of 
   flat values and logs of theta for many blocks of length
   THREADS_PER_BLOCK_ADD. The CPU will then sum the block

