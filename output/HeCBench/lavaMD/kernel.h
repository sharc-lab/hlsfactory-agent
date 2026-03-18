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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "./util/timer/timer.h"
#include "./util/num/num.h"
#include "./main.h"



// --- from kernel.h ---
void md ( const box_str* d_box_gpu,
    const FOUR_VECTOR* d_rv_gpu,
    const fp* d_qv_gpu,
    FOUR_VECTOR* d_fv_gpu,
    const fp alpha, 
    int dim_cpu_number_boxes) 
{

  FOUR_VECTOR rA_shared[100];
  FOUR_VECTOR rB_shared[100];
  fp qB_shared[100];

  int bx = _bid_x; 
  int tx = _tid_x;
  int wtx = tx;

  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------180
  //  DO FOR THE NUMBER OF BOXES
  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------180

  if(bx<dim_cpu_number_boxes){

    //------------------------------------------------------------------------------------------------------------------------------------------------------160
    //  Extract input parameters
    //------------------------------------------------------------------------------------------------------------------------------------------------------160

    // parameters
    fp a2 = 2*alpha*alpha;

    // home box
    int first_i;
    // (enable the line below only if wanting to use shared memory)

    // nei box
    int pointer;
    int k = 0;
    int first_j;
    int j = 0;
    // (enable the two lines below only if wanting to use shared memory)

    // common
    fp r2;
    fp u2;
    fp vij;
    fp fs;
    fp fxij;
    fp fyij;
    fp fzij;
    THREE_VECTOR d;

    //------------------------------------------------------------------------------------------------------------------------------------------------------160
    //  Home box
    //------------------------------------------------------------------------------------------------------------------------------------------------------160

    //----------------------------------------------------------------------------------------------------------------------------------140
    //  Setup parameters
    //----------------------------------------------------------------------------------------------------------------------------------140

    // home box - box parameters
    first_i = d_box_gpu[bx].offset;

    //----------------------------------------------------------------------------------------------------------------------------------140
    //  Copy to shared memory
    //----------------------------------------------------------------------------------------------------------------------------------140

    // (enable the section below only if wanting to use shared memory)
    // home box - shared memory
    while(wtx<NUMBER_PAR_PER_BOX){
      rA_shared[wtx] = d_rv_gpu[first_i+wtx];
      wtx = wtx + NUMBER_THREADS;
    }
    wtx = tx;

    // (enable the section below only if wanting to use shared memory)
    // synchronize threads  - not needed, but just to be safe for now

    //------------------------------------------------------------------------------------------------------------------------------------------------------160
    //  nei box loop
    //------------------------------------------------------------------------------------------------------------------------------------------------------160

    // loop over nei boxes of home box
    for (k=0; k<(1+d_box_gpu[bx].nn); k++){

      //----------------------------------------50
      //  nei box - get pointer to the right box
      //----------------------------------------50

      if(k==0){
        pointer = bx;                          // set first box to be processed to home box
      }
      else{
        pointer = d_box_gpu[bx].nei[k-1].number;              // remaining boxes are nei boxes
      }

      //----------------------------------------------------------------------------------------------------------------------------------140
      //  Setup parameters
      //----------------------------------------------------------------------------------------------------------------------------------140

      // nei box - box parameters
      first_j = d_box_gpu[pointer].offset;

      //----------------------------------------------------------------------------------------------------------------------------------140
      //  Setup parameters
      //----------------------------------------------------------------------------------------------------------------------------------140

      // (enable the section below only if wanting to use shared memory)
      // nei box - shared memory
      while(wtx<NUMBER_PAR_PER_BOX){
        rB_shared[wtx] = d_rv_gpu[first_j+wtx];
        qB_shared[wtx] = d_qv_gpu[first_j+wtx];
        wtx = wtx + NUMBER_THREADS;
      }
      wtx = tx;

      // (enable the section below only if wanting to use shared memory)
      // synchronize threads because in next section each thread accesses data brought in by different threads here

      //----------------------------------------------------------------------------------------------------------------------------------140
      //  Calculation
      //----------------------------------------------------------------------------------------------------------------------------------140

      // loop for the number of particles in the home box
      while(wtx<NUMBER_PAR_PER_BOX){

        // loop for the number of particles in the current nei box
        for (j=0; j<NUMBER_PAR_PER_BOX; j++){

          r2 = rA_shared[wtx].v + rB_shared[j].v - DOT(rA_shared[wtx],rB_shared[j]); 
          u2 = a2*r2;
          vij= exp(-u2);
          fs = 2*vij;
          d.x = rA_shared[wtx].x  - rB_shared[j].x;
          fxij=fs*d.x;
          d.y = rA_shared[wtx].y  - rB_shared[j].y;
          fyij=fs*d.y;
          d.z = rA_shared[wtx].z  - rB_shared[j].z;
          fzij=fs*d.z;
          d_fv_gpu[first_i+wtx].v +=  qB_shared[j]*vij;
          d_fv_gpu[first_i+wtx].x +=  qB_shared[j]*fxij;
          d_fv_gpu[first_i+wtx].y +=  qB_shared[j]*fyij;
          d_fv_gpu[first_i+wtx].z +=  qB_shared[j]*fzij;

        }

        // increment work thread index
        wtx = wtx + NUMBER_THREADS;

      }

      // reset work index
      wtx = tx;

      // synchronize after finishing force contributions from current nei box not to cause conflicts when starting next box

      //----------------------------------------------------------------------------------------------------------------------------------140
      //  Calculation END
      //----------------------------------------------------------------------------------------------------------------------------------140

    }

    //------------------------------------------------------------------------------------------------------------------------------------------------------160
    //  nei box loop END
    //------------------------------------------------------------------------------------------------------------------------------------------------------160

  }

}


// --- from main.h ---
//===============================================================================================================================================================================================================200
//	DEFINE / INCLUDE
//===============================================================================================================================================================================================================200

#define fp float

#define NUMBER_PAR_PER_BOX 100							// keep this low to allow more blocks that share shared memory to run concurrently, code does not work for larger than 110, more speedup can be achieved with larger number and no shared memory used

//#define NUMBER_THREADS 128								// this should be roughly equal to NUMBER_PAR_PER_BOX for best performance
// Parameterized work group size
#ifdef RD_WG_SIZE_0_0
        #define NUMBER_THREADS RD_WG_SIZE_0_0
#elif defined(RD_WG_SIZE_0)
        #define NUMBER_THREADS RD_WG_SIZE_0
#elif defined(RD_WG_SIZE)
        #define NUMBER_THREADS RD_WG_SIZE
#else
        #define NUMBER_THREADS 128
#endif

#define DOT(A,B) ((A.x)*(B.x)+(A.y)*(B.y)+(A.z)*(B.z))	// STABLE

//===============================================================================================================================================================================================================200
//	STRUCTURES
//===============================================================================================================================================================================================================200

typedef struct
{
	fp x, y, z;

} THREE_VECTOR;

typedef struct
{
	fp v, x, y, z;

} FOUR_VECTOR;

typedef struct nei_str
{

	// neighbor box
	int x, y, z;
	int number;
	long offset;

} nei_str;

typedef struct box_str
{

	// home box
	int x, y, z;
	int number;
	long offset;

	// neighbor boxes
	int nn;
	nei_str nei[26];

} box_str;

typedef struct par_str
{

	fp alpha;

} par_str;

typedef struct dim_str
{

	// input arguments
	int cur_arg;
	int arch_arg;
	int cores_arg;
	int boxes1d_arg;

	// system memory
	long number_boxes;
	long box_mem;
	long space_elem;
	long space_mem;
	long space_mem2;

} dim_str;

//===============================================================================================================================================================================================================200
//	FUNCTION PROTOTYPES
//===============================================================================================================================================================================================================200

int 
main(	int argc, 
		char *argv []);

//===============================================================================================================================================================================================================200
//	END
//===============================================================================================================================================================================================================200

