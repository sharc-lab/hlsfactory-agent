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

// --- from KeccakF.cu ---
/*
   GPU Implementation of Keccak by Guillaume Sevestre, 2010

   This code is hereby put in the public domain.
   It is given as is, without any guarantee.
 */

#include <stdio.h>
#include <stdlib.h>


// 22 rounds constants
// Study constant memory best placement with Cuda (textures ?)

const tKeccakLane KeccakF_RoundConstantsCPU[22] =
{
  (tKeccakLane)0x00000001 ,
  (tKeccakLane)0x00008082 ,
  (tKeccakLane)0x0000808a ,
  (tKeccakLane)0x80008000 ,
  (tKeccakLane)0x0000808b ,
  (tKeccakLane)0x80000001 ,
  (tKeccakLane)0x80008081 ,
  (tKeccakLane)0x00008009 ,
  (tKeccakLane)0x0000008a ,
  (tKeccakLane)0x00000088 ,
  (tKeccakLane)0x80008009 ,
  (tKeccakLane)0x8000000a ,
  (tKeccakLane)0x8000808b ,
  (tKeccakLane)0x0000008b ,
  (tKeccakLane)0x00008089 ,
  (tKeccakLane)0x00008003 ,
  (tKeccakLane)0x00008002 ,
  (tKeccakLane)0x00000080 ,
  (tKeccakLane)0x0000800a ,
  (tKeccakLane)0x8000000a ,
  (tKeccakLane)0x80008081 ,
  (tKeccakLane)0x00008080
};

//INFO It could be more optimized to use unsigned char on an 8-bit CPU
const unsigned int KeccakF_RotationConstants[25] =
{
  // 1,  3,  6, 10, 15, 21, 28, 36, 45, 55,  2, 14, 27, 41, 56,  8, 25, 43, 62, 18, 39, 61, 20, 44
  1,  3,  6, 10, 15, 21, 28, 4, 13, 23,  2, 14, 27, 9, 24,  8, 25, 11, 30, 18,  7,  29 , 20, 12
};

//INFO It could be more optimized to use unsigned char on an 8-bit CPU
const unsigned int KeccakF_PiLane[25] =
{
  10,  7, 11, 17, 18,  3,  5, 16,  8, 21, 24,  4, 15, 23, 19, 13, 12,  2, 20, 14, 22,  9,  6,  1
};

//INFO It could be more optimized to use unsigned char on an 8-bit CPU
const unsigned int KeccakF_Mod5[10] =
{
  0, 1, 2, 3, 4, 0, 1, 2, 3, 4
};



// Absorb blocks in top of tree keccak hash function
// inBuffer supposed to have block_number * output_block_size of data

//**************************
//Functions on Keccak state (seroize, print, compare)
//**************************



// print first 256 bits : output of Keccak hash

// Test equality of Keccak States


// --- from KeccakTreeCPU.cu ---
/*
   GPU Implementation of Keccak by Guillaume Sevestre, 2010

   This code is hereby put in the public domain.
   It is given as is, without any guarantee.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



// Implement a second stage on treehashing
// Use output of 2x OUTPUT_BLOCK_SIZE_B size to respect conditions for soundness of Treehashing



// --- from KeccakTreeGPU.cu ---
/*
   GPU Implementation of Keccak by Guillaume Sevestre, 2010

   This code is hereby put in the public domain.
   It is given as is, without any guarantee.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/*GPU constants
  tKeccakLane KeccakF_RoundConstants[22] =
  {
  (tKeccakLane)0x00000001 ,
  (tKeccakLane)0x00008082 ,
  (tKeccakLane)0x0000808a ,
  (tKeccakLane)0x80008000 ,
  (tKeccakLane)0x0000808b ,
  (tKeccakLane)0x80000001 ,
  (tKeccakLane)0x80008081 ,
  (tKeccakLane)0x00008009 ,
  (tKeccakLane)0x0000008a ,
  (tKeccakLane)0x00000088 ,
  (tKeccakLane)0x80008009 ,
  (tKeccakLane)0x8000000a ,
  (tKeccakLane)0x8000808b ,
  (tKeccakLane)0x0000008b ,
  (tKeccakLane)0x00008089 ,
  (tKeccakLane)0x00008003 ,
  (tKeccakLane)0x00008002 ,
  (tKeccakLane)0x00000080 ,
  (tKeccakLane)0x0000800a ,
  (tKeccakLane)0x8000000a ,
  (tKeccakLane)0x80008081 ,
  (tKeccakLane)0x00008080
  };
 */

//host constants
tKeccakLane KeccakF_RoundConstants_h[22] =
{
  (tKeccakLane)0x00000001 ,
  (tKeccakLane)0x00008082 ,
  (tKeccakLane)0x0000808a ,
  (tKeccakLane)0x80008000 ,
  (tKeccakLane)0x0000808b ,
  (tKeccakLane)0x80000001 ,
  (tKeccakLane)0x80008081 ,
  (tKeccakLane)0x00008009 ,
  (tKeccakLane)0x0000008a ,
  (tKeccakLane)0x00000088 ,
  (tKeccakLane)0x80008009 ,
  (tKeccakLane)0x8000000a ,
  (tKeccakLane)0x8000808b ,
  (tKeccakLane)0x0000008b ,
  (tKeccakLane)0x00008089 ,
  (tKeccakLane)0x00008003 ,
  (tKeccakLane)0x00008002 ,
  (tKeccakLane)0x00000080 ,
  (tKeccakLane)0x0000800a ,
  (tKeccakLane)0x8000000a ,
  (tKeccakLane)0x80008081 ,
  (tKeccakLane)0x00008080
};

// Device (GPU) Keccak-f function implementation
// unrolled
//end unrolled

//Host Keccak-f function (pb with using the same constants between host and device) 
//unrolled
//end unrolled

//Keccak final node hashing results of previous nodes in sequential mode

//************************************************************************
//kernel implementaing hash function, hashing NB_INPUT_BLOCK (of 256 bits)
//

//********************************************************************************

//************************
//First Tree mode
//data to be hashed is in h_inBuffer
//output chaining values hashes are copied to h_outBuffer
//************************


// --- from Test.cu ---
/*
   GPU Implementation of Keccak by Guillaume Sevestre, 2010

   This code is hereby put in the public domain.
   It is given as is, without any guarantee.

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>


// choose 8 for fast execution 
#define IMAX 8 // 1600 //2400 // 1600 for high speed mesures // iteration for speed mesure loops

tKeccakLane Kstate_cpu[25];
tKeccakLane Kstate_gpu[25];

//debug print function






// --- from main.cu ---
/*
GPU Implementation of Keccak by Guillaume Sevestre, 2010

This code is hereby put in the public domain.
It is given as is, without any guarantee.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>




// --- from KeccakF.h ---
/*
GPU Implementation of Keccak by Guillaume Sevestre, 2010

This code is hereby put in the public domain.
It is given as is, without any guarantee.
*/

#ifndef KECCAKF_H_INCLUDED
#define KECCAKF_H_INCLUDED


#define cKeccakNumberOfRounds   22 //22

#define ROL32(a, offset) ( ( (a) << (offset) ) ^ ( (a) >>(32-offset) ) )

//implementation of Keccak function on CPU
void KeccakF( tKeccakLane * state );

//implementation of Keccak function on CPU, unrolled
void KeccakF_CPU( tKeccakLane * state );

//set the state to zero
void zeroize( tKeccakLane * state );

//Keccak final node hashing results of previous nodes in sequential mode
// inBuffer supposed to have block_number * output_block_size of data
void Keccak_top(tKeccakLane * Kstate, tKeccakLane *inBuffer , int block_number);

//test equility of 2 keccak states
int isEqual_KS(tKeccakLane * Ks1, tKeccakLane * Ks2);

//print functions
void print_KS(tKeccakLane * state);
void print_KS_256(tKeccakLane * state);

#endif // KECCAKF_H_INCLUDED


// --- from KeccakTree.h ---
/*
GPU Implementation of Keccak by Guillaume Sevestre, 2010

This code is hereby put in the public domain.
It is given as is, without any guarantee.
*/

#ifndef KECCAKTREE_H_INCLUDED
#define KECCAKTREE_H_INCLUDED

#define NB_THREADS 64  // 96   // 192 // Numbers of threads PER BLOCK MUST BE a multiple of NB_SNCD_STAGE_NODES 
									//MUST BE > 8 for streamcipher mode 

#define NB_THREADS_BLOCKS 64 //  64 //   32 

#define NB_STREAMS 2 //  4  // 2 MUST DIVIDE NB_THREADS_BLOCKS

#define INPUT_BLOCK_SIZE_B 32   // 256 bits in : 32 Bytes MUST BE multiple of 4 
#define OUTPUT_BLOCK_SIZE_B 32  // 256 bits out of each keccak hash MUST BE multiple of 4 
#define NB_INPUT_BLOCK 1024   // 128  // 64   number of input block of 256 bits

// 2 stage Treehash
#define NB_SCND_STAGE_THREADS 16 // MUST DIVIDE NB_THREADS  
#define NB_INPUT_BLOCK_SNCD_STAGE  2*NB_THREADS/NB_SCND_STAGE_THREADS //

//StreamCipher
#define SC_NB_OUTPUT_BLOCK 64 // number of output blocks in stream cipher mode

#endif // KECCAKTREE_H_INCLUDED


// --- from KeccakTreeCPU.h ---
/*
GPU Implementation of Keccak by Guillaume Sevestre, 2010

This code is hereby put in the public domain.
It is given as is, without any guarantee.
*/

#ifndef KECCAKTREECPU_H_INCLUDED
#define KECCAKTREECPU_H_INCLUDED


//Implement Tree hash mode 1 on CPU
//data to be hashed is present in inBuffer
//output result is in outBuffer
void KeccakTreeCPU(tKeccakLane * inBuffer, tKeccakLane * outBuffer);

#endif // KECCAKTREECPU_H_INCLUDED


// --- from KeccakTreeGPU.h ---
/*
GPU Implementation of Keccak by Guillaume Sevestre, 2010

This code is hereby put in the public domain.
It is given as is, without any guarantee.
*/

#ifndef KECCAKTREEGPU_H_INCLUDED
#define KECCAKTREEGPU_H_INCLUDED


//************************
//First Tree mode
//data to be hashed is in h_inBuffer
//output chaining values hashes are copied to h_outBuffer
//************************

void KeccakTreeGPU(tKeccakLane * h_inBuffer, tKeccakLane * d_inBuffer, 
                   tKeccakLane * h_outBuffer, tKeccakLane * d_outBuffer, 
                   tKeccakLane * d_KeccakF_RoundConstants);

//error function
void  checkCUDAError(const char *msg);

#endif // KECCAKTREEGPU_H_INCLUDED


// --- from KeccakTypes.h ---
/*
GPU Implementation of Keccak by Guillaume Sevestre, 2010

This code is hereby put in the public domain.
It is given as is, without any guarantee.
*/
#ifndef KECCAKTYPES_H_INCLUDED
#define KECCAKTYPES_H_INCLUDED

typedef unsigned int  UINT32 ;

typedef unsigned int tKeccakLane ;

#endif // KECCAKTYPES_H_INCLUDED


// --- from Test.h ---
/*
GPU Implementation of Keccak by Guillaume Sevestre, 2010

This code is hereby put in the public domain.
It is given as is, without any guarantee.
*/

#ifndef TEST_H_INCLUDED
#define TEST_H_INCLUDED

//********************
// Basic treehash mode
//********************

//test Tree hash mode 1 in CPU only
//integer in argument is to lower the workload for CPU only test (slower than GPU tests)
void TestCPU(int);

//Test Tree hash mode 1 with GPU and CPU
void TestGPU(void);

//Test Tree hash mode 1 , GPU and CPU, CPU computation overlapped with GPU computation
void TestGPU_OverlapCPU(void);

void TestGPU_Split(void);

//Test Tree hash mode 1 , GPU and CPU, GPU computation is overlapped with memory transfers (Host to device) 
void TestGPU_Stream(void);

//Test Tree hash mode 1 , GPU and CPU, GPU computation is overlapped with memory transfers , and with CPU computation
void TestGPU_Stream_OverlapCPU(void);

//use of mapped memory : untested, unsupported by authors hardware
void TestGPU_MappedMemory(void);

//*************
//2 stages hash
//*************
void TestCPU_2stg(int);

void TestGPU_2stg(void);

void TestGPU_2stg_Stream_OverlapCPU(void);

//***************************
//Keccak in StreamCipher mode
//***************************
void TestGPU_SCipher(void);

// Other function

//Empirically Test if all words in input data are taken into the hash function
void Test_Completness(void);

//print GPU device info
void Device_Info(void);

//print Tree hash mode params set in KeccakTree.h
void Print_Param(void);

//verify 
void Verify_results(void);

#endif // TEST_H_INCLUDED
