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

// --- from main.cu ---
/** Modifed version of knn-CUDA from https://github.com/vincentfpgarcia/kNN-CUDA
 * The modifications are
 *      removed texture memory usage
 *      removed split query KNN computation
 *      added feature extraction with bilinear interpolation
 *
 * Last modified by Christopher B. Choy <chrischoy@ai.stanford.edu> 12/23/2016
 */

// Includes
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <hip/hip_runtime.h>

// Constants used by the program
#define BLOCK_DIM 16

/**

/**
  * Gathers k-th smallest distances for each column of the distance matrix in
 * the top.
  *
  * @param dist        distance matrix
  * @param ind         index matrix
  * @param width       width of the distance matrix and of the index matrix
  * @param height      height of the distance matrix and of the index matrix
  * @param k           number of neighbors to consider
  */

/**

//-----------------------------------------------------------------------------------------------//
//                                   K-th NEAREST NEIGHBORS //
//-----------------------------------------------------------------------------------------------//
/**
  * K nearest neighbor algorithm
  * - Initialize CUDA
  * - Allocate device memory
  * - Copy point sets (reference and query points) from host to device memory
  * - Compute the distances + indexes to the k nearest neighbors for each query
 * point
  * - Copy distances from device to host memory
  *
  * @param ref_host      reference points ; pointer to linear matrix
  * @param ref_width     number of reference points ; width of the matrix
  * @param query_host    query points ; pointer to linear matrix
  * @param query_width   number of query points ; width of the matrix
  * @param height        dimension of points ; height of the matrices
  * @param k             number of neighbor to consider
  * @param dist_host     distances to k nearest neighbors ; pointer to linear
 * matrix
  * @param dist_host     indexes of the k nearest neighbors ; pointer to linear
 * matrix
  *
  */




/**
  * Example of use of kNN search CUDA.
  */
