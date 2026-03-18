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

// --- from cpu.cu ---
/*  Copyright (c) 2011-2016, Robert Wang, email: robertwgh (at) gmail.com
  All rights reserved. https://github.com/robertwgh/cuLDPC

  CUDA implementation of LDPC decoding algorithm.
Created:   10/1/2010
Revision:  08/01/2013
/4/20/2016 prepare for release on Github.
*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

// custom header file

//===================================
// Random info data generation
//===================================

//===================================
// BPSK modulation
//===================================

//===================================
// AWGN modulation
//===================================

//===================================
// calc LLRs
//===================================

//===================================
// parity check
//===================================

//===================================
// parity check
//===================================

//===================================
// encoding
//===================================


// --- from kernel.cu ---
/*  Copyright (c) 2011-2016, Robert Wang, email: robertwgh (at) gmail.com
    All rights reserved. https://github.com/robertwgh/cuLDPC

    CUDA implementation of LDPC decoding algorithm.

    The details of implementation can be found from the following papers:
    1. Wang, G., Wu, M., Sun, Y., & Cavallaro, J. R. (2011, June). A massively parallel implementation of QC-LDPC decoder on GPU. In Application Specific Processors (SASP), 2011 IEEE 9th Symposium on (pp. 82-85). IEEE.
    2. Wang, G., Wu, M., Yin, B., & Cavallaro, J. R. (2013, December). High throughput low latency LDPC decoding on GPU for SDR systems. In Global Conference on Signal and Information Processing (GlobalSIP), 2013 IEEE (pp. 1258-1261). IEEE.

    The current release is close to the GlobalSIP2013 paper.

Created:   10/1/2010
Revision:  08/01/2013
04/20/2016 prepare for release on Github.
11/26/2017 cleanup and comments by Albin Severinson, albin (at) severinson.org
 */


// Kernel 1

// Kernel_1

// Kernel 2: VNP processing

// Kernel: VNP processing for the last iteration.


// --- from main.cu ---
/*  Copyright (c) 2011-2016, Robert Wang, email: robertwgh (at) gmail.com
  All rights reserved. https://github.com/robertwgh/cuLDPC

  Implementation of LDPC decoding algorithm.

  The details of implementation can be found from the following papers:
  1. Wang, G., Wu, M., Sun, Y., & Cavallaro, J. R. (2011, June). A massively parallel implementation of QC-LDPC decoder on GPU. In Application Specific Processors (SASP), 2011 IEEE 9th Symposium on (pp. 82-85). IEEE.
  2. Wang, G., Wu, M., Yin, B., & Cavallaro, J. R. (2013, December). High throughput low latency LDPC decoding on GPU for SDR systems. In Global Conference on Signal and Information Processing (GlobalSIP), 2013 IEEE (pp. 1258-1261). IEEE.

  The current release is close to the GlobalSIP2013 paper. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <chrono>
#include <hip/hip_runtime.h>

float sigma ;
int *info_bin ;



// --- from LDPC.h ---
/*  Copyright (c) 2011-2016, Robert Wang, email: robertwgh (at) gmail.com
  All rights reserved. https://github.com/robertwgh/cuLDPC

  CUDA implementation of LDPC decoding algorithm.
Created:   10/1/2010
Revision:  08/01/2013
/4/20/2016 prepare for release on Github.
*/

#ifndef LDPC_H
#define LDPC_H

#define YES  1
#define NO  0

// LDPC decoder configurations
#define WIMAX  0
#define WIFI  1
#define MODE  WIMAX
#define MIN_SUM  YES    //otherwise, log-SPA

// Simulation parameters
#define NUM_SNR 1
static float snr_array[NUM_SNR] = {3.0f};
#define MIN_FER         2000000 //2000000
#define MIN_CODEWORD    2000 // 9000000
#define MAX_ITERATION 10

#define CW 2 // code words per macro codewords
#define MCW 40 // number of macro codewords
#define MAX_SIM 500

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//  The following settings are fixed.
//  They don't need to be changed during simulations.
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#if MODE == WIMAX
// WIMAX
#define Z        96 //1024//96
#define NON_EMPTY_ELMENT 7
#define NON_EMPTY_ELMENT_VNP  6
#else 
// 802.11n
#define Z        81
#define NON_EMPTY_ELMENT 8 //the maximum number of non-empty element in a row of H matrix, for 1944 bit 802.11n code, NON_EMPTY_ELMENT=8
#define NON_EMPTY_ELMENT_VNP  11
#endif

#define BLOCK_SIZE_X ((Z + 32 - 1)/ 32 * 32)
#define THREADS_PER_BLOCK  (BLOCK_SIZE_X * CW)

#define BLK_ROW      12
#define BLK_COL      24
#define HALF_BLK_COL  12

#define BLK_INFO    BLK_ROW
#define BLK_CODEWORD  BLK_COL

#define ROW        (Z*BLK_ROW)
#define COL        (Z*BLK_COL)
#define INFO_LEN    (BLK_INFO * Z)
#define CODEWORD_LEN  (BLK_CODEWORD * Z)

// the slots in the H matrix
#define H_MATRIX    288
// the slots in the compact H matrix
#define H_COMPACT1_ROW  BLK_ROW
#define H_COMPACT1_COL  NON_EMPTY_ELMENT
#define H_COMPACT1 (BLK_ROW * NON_EMPTY_ELMENT) //96 // 8*12

#define H_COMPACT2_ROW  BLK_ROW
#define H_COMPACT2_COL  BLK_COL

typedef struct
{
  int bit_error;
  int frame_error;
} error_result;

typedef struct
{
  char x;
  char y;
  char value;
  char valid;
} h_element;

// Extern function and variable definition
void structure_encode (int s [], int code [], int h[BLK_ROW][BLK_COL]);
void info_gen (int info_bin [], long seed);
void modulation (int code [], float trans []);
void awgn (float trans [], float recv [], long seed);
void llr_init (float llr [], float recv []);
int parity_check (float app[]);
error_result error_check (int info[], int hard_decision[]);

// Variable declaration
extern float sigma ;
extern int *info_bin ;
extern FILE * gfp;

#endif


// --- from matrix.h ---
/*	Copyright (c) 2011-2016, Robert Wang, email: robertwgh (at) gmail.com
	All rights reserved. https://github.com/robertwgh/cuLDPC
	
	CUDA implementation of LDPC decoding algorithm.
	Created: 	10/1/2010
	Revision:	08/01/2013
			/4/20/2016 prepare for release on Github.
*/

#ifndef LDPC_MATRIX
#define LDPC_MATRIX


#if MODE == WIMAX
// 802.16e base matrix
int h_base [BLK_ROW][BLK_COL] = {
	{-1, 94, 73, -1, -1, -1, -1, -1, 55, 83, -1, -1,  7,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 
	{-1, 27, -1, -1, -1, 22, 79,  9, -1, -1, -1,  12,-1,  0,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 
	{-1, -1, -1, 24, 22, 81, -1, 33, -1, -1, -1,  0, -1, -1,  0,  0, -1, -1, -1, -1, -1, -1, -1, -1}, 
	{61, -1, 47, -1, -1, -1, -1, -1, 65, 25, -1, -1, -1, -1, -1,  0,  0, -1, -1, -1, -1, -1, -1, -1}, 
	{-1, -1, 39, -1, -1, -1, 84, -1, -1, 41, 72, -1, -1, -1, -1, -1,  0,  0, -1, -1, -1, -1, -1, -1}, 
	{-1, -1, -1, -1, 46, 40, -1, 82, -1, -1, -1, 79,  0, -1, -1, -1, -1,  0,  0, -1, -1, -1, -1, -1}, 
	{-1, -1, 95, 53, -1, -1, -1, -1, -1, 14, 18, -1, -1, -1, -1, -1, -1, -1,  0,  0, -1, -1, -1, -1}, 
	{-1, 11, 73, -1, -1, -1,  2, -1, -1, 47, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0, -1, -1, -1}, 
	{12, -1, -1, -1, 83, 24, -1, 43, -1, -1, -1, 51, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0, -1, -1}, 
	{-1, -1, -1, -1, -1, 94, -1, 59, -1, -1, 70, 72, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0, -1}, 
	{-1, -1,  7, 65, -1, -1, -1, -1, 39, 49, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0}, 
	{43, -1, -1, -1, -1, 66, -1, 41, -1, -1, -1, 26,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0}
} ;

#else
// 802.11n base matrix	
int h_base [BLK_ROW][BLK_COL] = {
	{57, -1, -1, -1, 50, -1, 11, -1, 50, -1, 79, -1,  1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 
	{ 3, -1, 28, -1,  0, -1, -1, -1, 55,  7, -1, -1, -1,  0,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 
	{30, -1, -1, -1, 24, 37, -1, -1, 56, 14, -1, -1, -1, -1,  0,  0, -1, -1, -1, -1, -1, -1, -1, -1}, 
	{62, 53, -1, -1, 53, -1, -1,  3, 35, -1, -1, -1, -1, -1, -1,  0,  0, -1, -1, -1, -1, -1, -1, -1}, 
	{40, -1, -1, 20, 66, -1, -1, 22, 28, -1, -1, -1, -1, -1, -1, -1,  0,  0, -1, -1, -1, -1, -1, -1}, 
	{ 0, -1, -1, -1,  8, -1, 42, -1, 50, -1, -1,  8, -1, -1, -1, -1, -1,  0,  0, -1, -1, -1, -1, -1}, 
	{69, 79, 79, -1, -1, -1, 56, -1, 52, -1, -1, -1,  0, -1, -1, -1, -1, -1,  0,  0, -1, -1, -1, -1}, 
	{65, -1, -1, -1, 38, 57, -1, -1, 72, -1, 27, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0, -1, -1, -1}, 
	{64, -1, -1, -1, 14, 52, -1, -1, 30, -1, -1, 32, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0, -1, -1}, 
	{-1, 45, -1, 70,  0, -1, -1, -1, 77,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0, -1}, 
	{ 2, 56, -1, 57, 35, -1, -1, -1, -1, -1, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0}, 
	{24, -1, 61, -1, 60, -1, -1, 27, 51, -1, -1, 16,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0}
} ;
#endif

#endif
