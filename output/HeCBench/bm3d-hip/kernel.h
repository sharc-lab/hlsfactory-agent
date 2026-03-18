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
#ifndef BLOCK_DIM_Y
#define BLOCK_DIM_Y 1
#endif
#ifndef GRID_DIM_Y
#define GRID_DIM_Y 1
#endif

// --- from blockmatching.cu ---
#include <hip/hip_runtime.h>
#include "params.hpp"
#include "indices.hpp"

// Nearest lower power of 2

//Computes the squared difference between two numbers
template<typename T>

/*

/*
   Block-matching algorithm
   For each processed reference patch it finds maximaly N similar patches that pass the distance threshold and stores them to the g_stacks array. 
   It also returns the number of them for each reference patch in g_num_patches_in_stack.
   Used denoising parameters: n,k,N,T,p
Division: Kernel handles GRID_DIM_Y lines starting with the line passed in argument. Each block handles warpSize reference patches in line. 
Each thread process one reference patch. All the warps of a block process the same reference patches.
 */

extern "C" void run_block_matching(
    const  uchar* __restrict image, //Original image
    ushort* __restrict stacks,         //For each reference patch contains addresses of similar patches (patch is adressed by top left corner)
    uint* __restrict num_patches_in_stack,    //For each reference patch contains number of similar patches
    const uint2 image_dim,      //Image dimensions
    const uint2 stacks_dim,      //size of area where reference patches could be located
    const Params params,      //Denoising parameters
    const uint2 start_point,    //Address of the top-left reference patch of a batch
    const dim3 num_threads,  
    const dim3 num_blocks,
    const uint shared_memory_size
    )
{
  hipLaunchKernelGGL(block_matching, num_blocks, num_threads, shared_memory_size, 0, 
      image,
      stacks,
      num_patches_in_stack,
      image_dim,
      stacks_dim,
      params,
      start_point
      );          
}


// --- from dct8x8.cu ---
#include <hip/hip_runtime.h>
#include "indices.hpp"
/*
 * Based on dct8x8_kernel2.cu provided in CUDA samples form NVIDIA Corporation.
 *
 * Provide functions to compute many 2D DCT and 2D IDCT of size 8x8 
*/

#define C_a 1.387039845322148f //!< a = (2^0.5) * cos(    pi / 16);  Used in forward and inverse DCT.  
#define C_b 1.306562964876377f //!< b = (2^0.5) * cos(    pi /  8);  Used in forward and inverse DCT.  
#define C_c 1.175875602419359f //!< c = (2^0.5) * cos(3 * pi / 16);  Used in forward and inverse DCT.  
#define C_d 0.785694958387102f //!< d = (2^0.5) * cos(5 * pi / 16);  Used in forward and inverse DCT.  
#define C_e 0.541196100146197f //!< e = (2^0.5) * cos(3 * pi /  8);  Used in forward and inverse DCT.  
#define C_f 0.275899379282943f //!< f = (2^0.5) * cos(7 * pi / 16);  Used in forward and inverse DCT.  

/**
*  Normalization constant that is used in forward and inverse DCT
*/
#define C_norm 0.3535533905932737f // 1 / (8^0.5)

#define BLOCK_SIZE          8

/**
*  Width of macro-block
*/
#define KER2_BLOCK_WIDTH          128

/**
*  Height of macro-block
*/
#define KER2_BLOCK_HEIGHT         8

/**

/**
**************************************************************************
*  Performs in-place IDCT of vector of 8 elements.
*
* \param Vect0          [IN/OUT] - Pointer to the first element of vector
* \param Step           [IN/OUT] - Value to add to ptr to access other elements
*
* \return None
*/

/**
**************************************************************************
*  Performs 8x8 block-wise Forward Discrete Cosine Transform of the given
*  image plane and outputs result to the array of coefficients. 2nd implementation.
*  This kernel is designed to process image by blocks of blocks8x8 that
*  utilizes maximum warps capacity, assuming that it is enough of 8 threads
*  per block8x8.
*
* \param SrcDst                     [OUT] - Coefficients plane
* \param ImgStride                  [IN] - Stride of SrcDst
*
* \return None
*/


/**
**************************************************************************
*  Performs 8x8 block-wise Inverse Discrete Cosine Transform of the given
*  coefficients plane and outputs result to the image. 2nd implementation.
*  This kernel is designed to process image by blocks of blocks8x8 that
*  utilizes maximum warps capacity, assuming that it is enough of 8 threads
*  per block8x8.
*
* \param SrcDst                     [OUT] - Coefficients plane
* \param ImgStride                  [IN] - Stride of SrcDst
*
* \return None
*/


extern "C" void run_DCT2D8x8(  
  float * __restrict transformed_stacks,
  const float * __restrict gathered_stacks,
  const uint size,
  const dim3 num_threads,
  const dim3 num_blocks)
{
  hipLaunchKernelGGL(IDCT2D8x8, num_blocks, num_threads, 0, 0, gathered_stacks, transformed_stacks, size);
}


// --- from filtering.cu ---
#include <float.h>
#include <stdio.h>
#include <hip/hip_runtime.h>

#include "indices.hpp"
#include "params.hpp"

// Kernels used for collaborative filtering and aggregation

//Sum the passed values in a warp to the first thread of this warp.
template<typename T>

//Sum the passed values in a block to the first thread of a block.
template<typename T>

//Returns absolute value of the passed real number raised to the power of two
inline

//Integer logarithm base 2.
template <typename IntType>

//Orthogonal transformation.
template <typename T>

//Fast Walsh-Hadamard transform.
template <typename T>

//Based on blockIdx it computes the addresses to the arrays in global memory

/*
Gather patches form image based on matching stored in 3D array stacks
Used parameters: p,k,N
Division: One block handles one patch_stack, threads match to the pixels of a patch
*/

/*
1) Do the Walsh-Hadamard 1D transform on the z axis of 3D stack. 
2) Treshold every pixel and count the number of non-zero coefficients
3) Do the inverse Walsh-Hadamard 1D transform on the z axis of 3D stack.
Used parameters: L3D,N,k,p
Division: Each block delas with one transformed patch stack. (number of threads in block should be k*k)
*/

/*
Fills two buffers: numerator and denominator in order to compute weighted average of pixels
Used parameters: k,N,p
Division: Each block delas with one transformed patch stack.
*/

/*
Divide numerator with denominator and round result to image_o
*/

extern "C" void run_get_block(
  const uint2 start_point,
  const uchar* __restrict image,
  const ushort* __restrict stacks,
  const uint* __restrict num_patches_in_stack,
  float* __restrict patch_stack,
  const uint2 image_dim,
  const uint2 stacks_dim,
  const Params params,
  const dim3 num_threads,
  const dim3 num_blocks)
{
  hipLaunchKernelGGL(get_block, num_blocks, num_threads, 0, 0, 
    start_point,
    image,
    stacks,
    num_patches_in_stack,
    patch_stack,
    image_dim,
    stacks_dim,
    params
  );
}

extern "C" void run_hard_treshold_block(
  const uint2 start_point,
  float* __restrict patch_stack,
  float* __restrict w_P,
  const uint* __restrict num_patches_in_stack,
  const uint2 stacks_dim,
  const Params params,
  const uint sigma,
  const dim3 num_threads,
  const dim3 num_blocks,
  const uint shared_memory_size)
{
  hipLaunchKernelGGL(hard_treshold_block, num_blocks, num_threads, shared_memory_size, 0, 
    start_point,
    patch_stack,
    w_P,
    num_patches_in_stack,
    stacks_dim,
    params,
    sigma
  );
}

extern "C" void run_aggregate_block(
  const uint2 start_point,
  const float* __restrict patch_stack,  
  const float* __restrict w_P,
  const ushort* __restrict stacks,
  const float* __restrict kaiser_window,
  float* __restrict numerator,
  float* __restrict denominator,
  const uint* __restrict num_patches_in_stack,
  const uint2 image_dim,
  const uint2 stacks_dim,
  const Params params,
  const dim3 num_threads,
  const dim3 num_blocks)
{
  hipLaunchKernelGGL(aggregate_block, num_blocks, num_threads, 0, 0, 
    start_point,
    patch_stack,
    w_P,
    stacks,
    kaiser_window,
    numerator,
    denominator,
    num_patches_in_stack,
    image_dim,
    stacks_dim,
    params
  );
}

extern "C" void run_aggregate_final(
  const float* __restrict numerator,
  const float* __restrict denominator,
  const uint2 image_dim,
  uchar*__restrict  denoised_image,
  const dim3 num_threads,
  const dim3 num_blocks
)
{
  hipLaunchKernelGGL(aggregate_final, num_blocks, num_threads, 0, 0, 
    numerator,
    denominator,
    image_dim,
    denoised_image
  );  
}



// --- from main.cu ---
#include <iostream>
#include <string>
#include <chrono>

#include "bm3d.hpp"
#define cimg_display 0
#include "CImg.h"

// Repeat the execution of kernels 100 times
#define REPEAT 100

// Adjust the size of the total shared local memory for different GPUs
// e.g. 48KB on P100
#define TOTAL_SLM     48*1024

// Adjust the thread block size of the block matching kernel for different GPUs. 
// The maximum thread block size is 32 * MAX_NUM_WARPS
#define MAX_NUM_WARPS 16u

using namespace cimg_library;

