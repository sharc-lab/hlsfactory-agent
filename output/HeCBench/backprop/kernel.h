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

// --- from backprop.cu ---
/*
 ******************************************************************
 * HISTORY
 * 15-Oct-94  Jeff Shufelt (js), Carnegie Mellon University
 *	Prepared for 15-681, Fall 1994.
 * Modified by Shuai Che
 ******************************************************************
 */

//#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ABS(x)          (((x) > 0.0) ? (x) : (-(x)))

#define fastcopy(to,from,len)\
{\
  char *_to,*_from;\
  int _i,_l;\
  _to = (char *)(to);\
  _from = (char *)(from);\
  _l = (len);\
  //float m;
  //x = -x;
  //m = 1 + x + x*x/2 + x*x*x/6 + x*x*x*x/24 + x*x*x*x*x/120;
  //return(1.0 / (1.0 + m));
  float *new_t;

  new_t = (float *) malloc ((unsigned) (n * sizeof (float)));
  int i;
  float **new_t;

  new_t = (float **) malloc ((unsigned) (m * sizeof (float *)));


  int i, j;

}




BPNN *bpnn_internal_create(int n_in, int n_hidden, int n_out)
{
  BPNN *newnet;

  newnet = (BPNN *) malloc (sizeof (BPNN));

  newnet->input_n = n_in;
  newnet->hidden_n = n_hidden;
  newnet->output_n = n_out;
  newnet->input_units = alloc_1d_dbl(n_in + 1);
  newnet->hidden_units = alloc_1d_dbl(n_hidden + 1);
  newnet->output_units = alloc_1d_dbl(n_out + 1);

  newnet->hidden_delta = alloc_1d_dbl(n_hidden + 1);
  newnet->output_delta = alloc_1d_dbl(n_out + 1);
  newnet->target = alloc_1d_dbl(n_out + 1);

  newnet->input_weights = alloc_2d_dbl(n_in + 1, n_hidden + 1);
  newnet->hidden_weights = alloc_2d_dbl(n_hidden + 1, n_out + 1);

  newnet->input_prev_weights = alloc_2d_dbl(n_in + 1, n_hidden + 1);
  newnet->hidden_prev_weights = alloc_2d_dbl(n_hidden + 1, n_out + 1);

  int n1, n2, i;

  n1 = net->input_n;
  n2 = net->hidden_n;

  free((char *) net->input_units);
  free((char *) net->hidden_units);
  free((char *) net->output_units);

  free((char *) net->hidden_delta);
  free((char *) net->output_delta);
  free((char *) net->input_weights);
  free((char *) net->hidden_weights);
  free((char *) net->hidden_prev_weights);


  BPNN *newnet;

  newnet = bpnn_internal_create(n_in, n_hidden, n_out);

#ifdef INITZERO
  bpnn_zero_weights(newnet->input_weights, n_in, n_hidden);
#else
  bpnn_randomize_weights(newnet->input_weights, n_in, n_hidden);
#endif
  bpnn_randomize_weights(newnet->hidden_weights, n_hidden, n_out);
  bpnn_zero_weights(newnet->input_prev_weights, n_in, n_hidden);
  bpnn_zero_weights(newnet->hidden_prev_weights, n_hidden, n_out);
  bpnn_randomize_row(newnet->target, n_out);
  float sum;
  int j, k;

  /*** Set up thresholding unit ***/
  l1[0] = 1.0;
#ifdef OPEN
}







BPNN *bpnn_read(char *filename)
{
  char *mem;
  BPNN *new_t;
  int fd, n1, n2, n3, i, j, memcnt;


  printf("Reading '%s'\n", filename);  //fflush(stdout);

  read(fd, (char *) &n1, sizeof(int));
  read(fd, (char *) &n2, sizeof(int));
  read(fd, (char *) &n3, sizeof(int));
  new_t = bpnn_internal_create(n1, n2, n3);

  printf("'%s' contains a %dx%dx%d network\n", filename, n1, n2, n3);
  printf("Reading input weights...");  //fflush(stdout);

  memcnt = 0;
  mem = (char *) malloc ((unsigned) ((n1+1) * (n2+1) * sizeof(float)));
  free(mem);

  printf("Done\nReading hidden weights...");  //fflush(stdout);

  memcnt = 0;
  mem = (char *) malloc ((unsigned) ((n2+1) * (n3+1) * sizeof(float)));
  free(mem);
  close(fd);

  printf("Done\n");  //fflush(stdout);

  bpnn_zero_weights(new_t->input_prev_weights, n1, n2);
  bpnn_zero_weights(new_t->hidden_prev_weights, n2, n3);

  return (new_t);
}


// --- from facetrain.cu ---
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern char *strcpy();
extern void exit();

int layer_size = 0;




// --- from imagenet.cu ---

#include <stdio.h>
#include <stdlib.h>

extern int layer_size;

void load(BPNN *net)
  //BPNN *net;
{
  float *units;
  int nr, i, k;

  nr = layer_size;

  units = net->input_units;

  k = 1;
}


// --- from main.cu ---
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>


// cuda kernels



unsigned int num_threads = 0;
unsigned int num_blocks = 0;

////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////



// --- from backprop.h ---
#ifndef _BACKPROP_H_
#define _BACKPROP_H_

#define BIGRND 0x7fffffff
#define THREADS 256
#define WIDTH 16  // shared memory width  
#define HEIGHT 16 // shared memory height
#define BLOCK_SIZE 16

#define ETA 0.3f       //eta value
#define MOMENTUM 0.3f  //momentum value
#define NUM_THREAD 4  //OpenMP threads

#define WM(i, j)   weight_matrix[(j) + (i) * WIDTH]

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

//void bpnn_initialize();
void bpnn_initialize(int seed);
BPNN *bpnn_create(int n_in, int n_hidden, int n_out);
void bpnn_free(BPNN *net);
//BPNN *bpnn_create();
//void bpnn_free();
void bpnn_train(BPNN *net, float *eo, float *eh);
//void bpnn_train();
//void bpnn_feedforward();
void bpnn_feedforward(BPNN *net);
void bpnn_save(BPNN *net, char *filename);
//void bpnn_save();
//BPNN *bpnn_read();
BPNN *bpnn_read(char *filename);
void load(BPNN *net);
int bpnn_train_kernel(BPNN *net, float *eo, float *eh);
void bpnn_layerforward(float *l1, float *l2, float **conn, int n1, int n2);
void bpnn_output_error(float *delta, float *target, float *output, int nj, float *err);
void bpnn_hidden_error(float *delta_h, int nh, float *delta_o, int no, float **who, float *hidden, float *err); 
void bpnn_adjust_weights(float *delta, int ndelta, float *ly, int nly, float **w, float **oldw);
void setup(int argc, char** argv);
float **alloc_2d_dbl(int m, int n);
float squash(float x);

#endif


// --- from bpnn_adjust_weights.h ---
void kernel_adjust_weights (
  const float* ly, 
       float * w, 
  const float* delta, 
        float* oldw, 
  const int hid)
{
  int by = _bid_y; 
  int tx = _tid_x; 
  int ty = _tid_y;

  int index =  ( hid + 1 ) * HEIGHT * by + ( hid + 1 ) * ty + tx + 1 + ( hid + 1 ) ;  
  int index_y = HEIGHT * by + ty + 1;
  int index_x = tx + 1;

  w[index] += ((ETA * delta[index_x] * ly[index_y]) + (MOMENTUM * oldw[index]));
  oldw[index] = ((ETA * delta[index_x] * ly[index_y]) + (MOMENTUM * oldw[index]));

  if (ty == 0 && by ==0){
    w[index_x] += ((ETA * delta[index_x]) + (MOMENTUM * oldw[index_x]));
    oldw[index_x] = ((ETA * delta[index_x]) + (MOMENTUM * oldw[index_x]));
  }
}



// --- from bpnn_layerforward.h ---
void kernel_layerforward(
  const float* input,
        float* input_weights,
        float* hidden_partial_sum,
  const int hid) 
{
  float input_node[HEIGHT];
  float weight_matrix[HEIGHT * WIDTH];

  // GRID_DIM_Y << GRID_DIM_X
  int by = _bid_y; 
  int tx = _tid_x; 
  int ty = _tid_y;

  int index = ( hid + 1 ) * HEIGHT * by + ( hid + 1 ) * ty + tx + 1 + ( hid + 1 ) ;  

  int index_in = HEIGHT * by + ty + 1;

  if ( tx == 0 )
    input_node[ty] = input[index_in] ;

  weight_matrix[ty * WIDTH + tx] =  input_weights[index];

  weight_matrix[ty * WIDTH + tx]= weight_matrix[ty * WIDTH + tx] * input_node[ty];

  for ( int i = 1 ; i <= HEIGHT ; i=i*2){
    int power_two = i; 

    if( ty % power_two == 0 )
      weight_matrix[ty * WIDTH + tx]= weight_matrix[ty * WIDTH + tx] + weight_matrix[(ty + power_two/2)* WIDTH + tx];

  }

  input_weights[index] =  weight_matrix[ty * WIDTH + tx];

  if ( tx == 0 ) {
    hidden_partial_sum[by * hid + ty] = weight_matrix[tx* WIDTH + ty];
  }
}
