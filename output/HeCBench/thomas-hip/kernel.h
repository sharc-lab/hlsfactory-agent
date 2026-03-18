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

// --- from cuThomasBatch.cu ---
/**
 *
 *  @file cuThomasBatch.cu
 *
 *  @brief cuThomasBatch kernel implementaion.
 *
 *  cuThomasBatch is a software package provided by
 *  Barcelona Supercomputing Center - Centro Nacional de Supercomputacion
 *
 *  @author Ivan Martinez-Perez ivan.martinez@bsc.es
 *  @author Pedro Valero-Lara   pedro.valero@bsc.es
 *
 **/

/**
 *
 *  @ingroup cuThomasBatch
 *  
 *  Solve a set of Tridiagonal linear systems:
 *
 *      A_ix_i = RHS_i, for all i = 0, ..., N
 *
 *      N = BATCHCOUNT
 *
 *  where A is a MxM tridiagonal matrix:
 *
 *      A_i = [ D_i[0]     U_i[1]    .        .    .          .       
 *              L_i[0]     D_i[1]    U_i[2]   .    .          .          
 *              .          L_i[1]    D_i[2]   .    .          .     
 *              .          .         L_i[2]   .    .          U_i[M-1] 
 *              .          .         .        .    L_i[M-2]   D_i[M-1] ]
 *
 *  Note that the elements of the inputs must be interleaved by following the
 *  next pattern for N (BATCHCOUNT) tridiagonal systems and M elements each:
 *
 *      D_0[0], D_1[0], ..., D_N[0], ..., D_0[M-1], D_1[M-1], ..., D_N[M-1]
 *
**/

/**
 *  
 *  @param[in]
 *  L           double *.
 *              L is a pointer to the lower-diagonal vector
 *          
 *  @param[in]
 *  D           double *.
 *              D is a pointer to the diagonal vector
 *
 *  @param[in,out]
 *  U           double *.
 *              U is a pointer to the uper-diagonal vector
 *
 *  @param[in,out]
 *  RHS         double *.    
 *              RHS is a pointer to the Right Hand Side vector
 *   
 *   
 *  @param[in]
 *  M           int.
 *              M specifies the number of elemets of the systems 
 *
 *  @param[in]
 *  BATCHCOUNT  int.
 *              BATCHCOUNT specifies to number of systems to be procesed
 **/



// --- from main.cu ---
#include <chrono>
#include <iostream>
#include <hip/hip_runtime.h>
#include "ThomasMatrix.hpp"
#include "utils.hpp"

// CPU kernel



// --- from cuThomasBatch.h ---
#include "hip/hip_runtime.h"
/**
 *
 *  @file cuThomasBatch.h
 *
 *  @brief cuThomasBatch kernel implementaion.
 *
 *  cuThomasBatch is a software package provided by
 *  Barcelona Supercomputing Center - Centro Nacional de Supercomputacion
 *
 *  @author Ivan Martinez-Perez ivan.martinez@bsc.es
 *  @author Pedro Valero-Lara   pedro.valero@bsc.es
 *
 **/

void cuThomasBatch(
            const double *L, const double *D, double *U, double *RHS,
            const int M,
            const int BATCHCOUNT);
