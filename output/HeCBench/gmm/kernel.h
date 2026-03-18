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

// --- from cluster.cu ---
/*

///////////////////////////////////////////////////////////////////////////////
// Validate command line arguments
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Print usage statement
///////////////////////////////////////////////////////////////////////////////






// Free the cluster data structures on host

// Free the cluster data structures on device

// Setup the cluster data structures on host

// Setup the cluster data structures on device





// --- from gaussian_kernel.cu ---
/*
 * CUDA Kernels for Expectation Maximization with Gaussian Mixture Models
 *
 * Author: Andrew Pangborn
 * 
 * Department of Computer Engineering
 * Rochester Institute of Technology
 */

#ifndef _TEMPLATE_KERNEL_H_
#define _TEMPLATE_KERNEL_H_


/*
 * Compute the multivariate mean of the FCS data
 */ 


// Inverts an NxN matrix 'data' stored as a 1D array in-place
// 'actualsize' is N
// Computes the log of the determinant of the origianl matrix in the process



/*
 * Computes the constant, pi, Rinv for each cluster
 * 
 * Needs to be launched with the number of blocks = number of clusters
 */

////////////////////////////////////////////////////////////////////////////////
//! @param fcs_data         FCS data: [num_events]
//! @param clusters         Clusters: [num_clusters]
//! @param num_dimensions   number of dimensions in an FCS event
//! @param num_events       number of FCS events
////////////////////////////////////////////////////////////////////////////////





/*
 * Means kernel
 * MultiGPU version, sums up all of the elements, but does not divide by N. 
 * This task is left for the host after combing results from multiple GPUs
 *
 * Should be launched with [M x D] grid
 */

/*
 * Computes the size of each cluster, N

/*
 * Computes the row and col of a square matrix based on the index into

/*


// --- from main.cu ---
/*
 * CUDA Expectation Maximization with Gaussian Mixture Models
 * Multi-GPU implemenetation using OpenMP
 *
 * Written By: Andrew Pangborn
 * 09/2009
 *
 * Department of Computer Engineering
 * Rochester Institute of Technology
 *
 */

// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h> // for clock(), clock_t, CLOCKS_PER_SEC
#include <stdlib.h>
#include <float.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////



// --- from readData.cu ---
/*
 *  Created on: Nov 4, 2008
 *      Author: Doug Roberts
 *      Modified by: Andrew Pangborn
 */

using namespace std;

float* readData(char* f, int* ndims, int* nevents);
float* readBIN(char* f, int* ndims, int* nevents);




// --- from gaussian.h ---
/*
 * Parameters file for gaussian mixture model based clustering application
 *
 * Written By: Andrew Pangborn
 * 09/2009
 *
 * Department of Computer Engineering
 * Rochester Institute of Technology
 *
 */

#include <stdio.h> // for FILE typedef

#ifndef GAUSSIAN_H
#define GAUSSIAN_H

#define PI  3.1415926535897931f
#define COVARIANCE_DYNAMIC_RANGE 1E6

// if 0, uses random, else picks events uniformly distributed in data set
#define UNIFORM_SEED 0

// if 1, removes data points such that the total is a multiple of 16*num_gpus
// Ensures memory alignment and therefore maximum performance
// set to 0 if losing data points is unacceptable
#define TRUNCATE 1

// Number of blocks per cluster for the E-step
#define	NUM_BLOCKS 24
#define NUM_THREADS_ESTEP 256 // should be a power of 2 for parallel reductions to work
#define NUM_THREADS_MSTEP 256 // should be a power of 2 for parallel reductions to work
#define NUM_DIMENSIONS 24
#define NUM_CLUSTERS_PER_BLOCK 6

// Which GPU to use, if more than 1
#define DEVICE 0

// Using only diagonal covariance matrix, thus all dimensions are considered independent
// Reduces computation complexity of some kernels by a factor of D
#define DIAG_ONLY 0

// Maximum number of iterations for the EM convergence loop
#define MAX_ITERS 200
#define MIN_ITERS 1

// Prints verbose output during the algorithm
// Enables the DEBUG macro
#define ENABLE_DEBUG 0

// Used to enable regular print outs (such as the Rissanen scores, clustering results)
// This should be enabled for general use and disabled for performance evaluations only
#define ENABLE_PRINT 1

// Used to enable output of cluster results to .results and .summary files
// Disable for performance testing
#define ENABLE_OUTPUT 1

// Used to enable EMUPRINT macro, this can only be used when compiled for
// in emulation mode. It is used to print out during cuda kernels
#define EMU 0

#if ENABLE_DEBUG
#define DEBUG(fmt, ...) printf(fmt, ## __VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

#if ENABLE_PRINT
#define PRINT(fmt, ...) printf(fmt, ## __VA_ARGS__)
#else
#define PRINT(fmt, ...)
#endif

#ifdef EMU
#define EMUPRINT(fmt, ...) printf(fmt, ## __VA_ARGS__)
#else
#define EMUPRINT(fmt, ...)
#endif

#define CUDA_SAFE_CALL( call) do {                                 \
    cudaError err = call;                                                    \
    if( cudaSuccess != err) {                                                \
        fprintf(stderr, "Cuda error in file '%s' in line %i : %s.\n",        \
            __FILE__, __LINE__, cudaGetErrorString( err) );                  \
             exit(EXIT_FAILURE);                                             \
    } } while (0)

typedef struct 
{
    // Key for array lengths
    //  N = number of events
    //  M = number of clusters
    //  D = number of dimensions
    float* N;        // expected # of pixels in cluster: [M]
    float* pi;       // probability of cluster in GMM: [M]
    float* constant; // Normalizing constant [M]
    float* avgvar;    // average variance [M]
    float* means;   // Spectral mean for the cluster: [M*D]
    float* R;      // Covariance matrix: [M*D*D]
    float* Rinv;   // Inverse of covariance matrix: [M*D*D]
    float* memberships; // Fuzzy memberships: [N*M]
} clusters_t;

int validateArguments(int argc, char** argv, int* num_clusters, FILE** infile, FILE** outfile);
void printUsage(char** argv);
#endif

