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
#ifndef BLOCK_DIM_Y
#define BLOCK_DIM_Y 1
#endif
#ifndef GRID_DIM_Y
#define GRID_DIM_Y 1
#endif

// --- from backprop_cuda.cu ---


// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

// includes, kernels

////////////////////////////////////////////////////////////////////////////////

extern "C"
void bpnn_layerforward(float *l1, float *l2, float **conn, int n1, int n2);

extern "C"
void bpnn_output_error(float *delta, float *target, float *output, int nj, float *err);

extern "C"
void bpnn_hidden_error(float *delta_h, int nh, float *delta_o, int no, float **who, float *hidden, float *err);

extern "C" 
void bpnn_adjust_weights(float *delta, int ndelta, float *ly, int nly, float **w, float **oldw);

extern "C"
int setup(int argc, char** argv);

extern "C"
float **alloc_2d_dbl(int m, int n);

extern "C"

unsigned int num_threads = 0;
unsigned int num_blocks = 0;

////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////

extern "C"


// --- from backprop_cuda_kernel.cu ---


#ifndef _BACKPROP_CUDA_KERNEL_H_
#define _BACKPROP_CUDA_KERNEL_H_

#include <stdio.h>
#include "math.h"


#endif 


// --- from backprop.h ---
#ifndef _BACKPROP_H_
#define _BACKPROP_H_

#define BIGRND 0x7fffffff

#define GPU
#define THREADS 256
#define WIDTH 16  // shared memory width  
#define HEIGHT 16 // shared memory height

#define ETA 0.3       //eta value
#define MOMENTUM 0.3  //momentum value
#define NUM_THREAD 4  //OpenMP threads

typedef struct {
  int input_n;                  /* number of input units */
  int hidden_n;                 /* number of hidden units */
  int output_n;                 /* number of output units */

  float *input_units;          /* the input units */
  float *hidden_units;         /* the hidden units */
  float *output_units;         /* the output units */

  float *hidden_delta;         /* storage for hidden unit error */
  float *output_delta;         /* storage for output unit error */

  float *target;               /* storage for target vector */

  float **input_weights;       /* weights from input to hidden layer */
  float **hidden_weights;      /* weights from hidden to output layer */

                                /*** The next two are for momentum ***/
  float **input_prev_weights;  /* previous change on input to hidden wgt */
  float **hidden_prev_weights; /* previous change on hidden to output wgt */
} BPNN;

/*** User-level functions ***/

void bpnn_initialize();

BPNN *bpnn_create();
void bpnn_free();

void bpnn_train();
void bpnn_feedforward();

void bpnn_save();
BPNN *bpnn_read();

#endif
