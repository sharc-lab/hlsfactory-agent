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

// --- from hotspot.cu ---
#include <chrono>

// Returns the current system time in microseconds



/* compute N time steps */




// --- from hotspot.h ---
#ifndef HOTSPOT_H
#define HOTSPOT_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#ifdef RD_WG_SIZE_0_0                                                            
        #define BLOCK_SIZE RD_WG_SIZE_0_0                                        
#elif defined(RD_WG_SIZE_0)                                                      
        #define BLOCK_SIZE RD_WG_SIZE_0                                          
#elif defined(RD_WG_SIZE)                                                        
        #define BLOCK_SIZE RD_WG_SIZE                                            
#else                                                                                    
        #define BLOCK_SIZE 16                                                            
#endif                                                                                   

#define STR_SIZE 256
# define EXPAND_RATE 2// add one iteration will extend the pyramid base by 2 per each borderline

/* maximum power density possible (say 300W for a 10mm x 10mm chip) */
#define MAX_PD	(3.0e6)
/* required precision in degrees */
#define PRECISION	0.001
#define SPEC_HEAT_SI 1.75e6
#define K_SI 100
/* capacitance fitting factor */
#define FACTOR_CHIP	0.5

#define MIN(a, b) ((a)<=(b) ? (a) : (b))

#define IN_RANGE(x, min, max)   ((x)>=(min) && (x)<=(max))

/* chip parameters */
const static float t_chip = 0.0005;
const static float chip_height = 0.016;
const static float chip_width = 0.016;
/* ambient temperature, assuming no package at all */
const static float amb_temp = 80.0;

void writeoutput(float *, int, int, char *);
void readinput(float *, int, int, char *);
void usage(int, char **);
void run(int, char **);

#endif


// --- from kernel.h ---
void calc_temp(
    int iteration,  //number of iteration
    const float * power,   //power input
    const float * temp_src,//temperature input/output
          float * temp_dst,//temperature input/output
    int grid_cols,  //Col of grid
    int grid_rows,  //Row of grid
    int border_cols,// border offset 
    int border_rows,// border offset
    float Cap,      //Capacitance
    float Rx, 
    float Ry, 
    float Rz, 
    float step)
{

  float temp_on_device[BLOCK_SIZE][BLOCK_SIZE];
  float power_on_device[BLOCK_SIZE][BLOCK_SIZE];
  float temp_t[BLOCK_SIZE][BLOCK_SIZE]; // temparary temperature result

  float amb_temp = 80.0f;
  float step_div_Cap;
  float Rx_1,Ry_1,Rz_1;

  int bx = _bid_x;
  int by = _bid_y;

  int tx = _tid_x;
  int ty = _tid_y;

  step_div_Cap = step/Cap;

  Rx_1 = 1.f/Rx;
  Ry_1 = 1.f/Ry;
  Rz_1 = 1.f/Rz;

  // each block finally computes result for a small block
  // after N iterations. 
  // it is the non-overlapping small blocks that cover 
  // all the input data

  // calculate the small block size
  int small_block_rows = BLOCK_SIZE-iteration*2;//EXPAND_RATE
  int small_block_cols = BLOCK_SIZE-iteration*2;//EXPAND_RATE

  // calculate the boundary for the block according to 
  // the boundary of its small block
  int blkY = small_block_rows*by-border_rows;
  int blkX = small_block_cols*bx-border_cols;
  int blkYmax = blkY+BLOCK_SIZE-1;
  int blkXmax = blkX+BLOCK_SIZE-1;

  // calculate the global thread coordination
  int yidx = blkY+ty;
  int xidx = blkX+tx;

  // load data if it is within the valid input range
  int loadYidx=yidx, loadXidx=xidx;
  int index = grid_cols*loadYidx+loadXidx;

  if(IN_RANGE(loadYidx, 0, grid_rows-1) && IN_RANGE(loadXidx, 0, grid_cols-1)){
    temp_on_device[ty][tx] = temp_src[index];  // Load the temperature data from global memory to shared memory
    power_on_device[ty][tx] = power[index];// Load the power data from global memory to shared memory
  }

  // effective range within this block that falls within 
  // the valid range of the input data
  // used to rule out computation outside the boundary.
  int validYmin = (blkY < 0) ? -blkY : 0;
  int validYmax = (blkYmax > grid_rows-1) ? BLOCK_SIZE-1-(blkYmax-grid_rows+1) : BLOCK_SIZE-1;
  int validXmin = (blkX < 0) ? -blkX : 0;
  int validXmax = (blkXmax > grid_cols-1) ? BLOCK_SIZE-1-(blkXmax-grid_cols+1) : BLOCK_SIZE-1;

  int N = ty-1;
  int S = ty+1;
  int W = tx-1;
  int E = tx+1;

  N = (N < validYmin) ? validYmin : N;
  S = (S > validYmax) ? validYmax : S;
  W = (W < validXmin) ? validXmin : W;
  E = (E > validXmax) ? validXmax : E;

  bool computed;
  for (int i=0; i<iteration ; i++){ 
    computed = false;
    if( IN_RANGE(tx, i+1, BLOCK_SIZE-i-2) &&  \
        IN_RANGE(ty, i+1, BLOCK_SIZE-i-2) &&  \
        IN_RANGE(tx, validXmin, validXmax) && \
        IN_RANGE(ty, validYmin, validYmax) ) {
      computed = true;
      temp_t[ty][tx] =   temp_on_device[ty][tx] + step_div_Cap * (power_on_device[ty][tx] + 
          (temp_on_device[S][tx] + temp_on_device[N][tx] - 2.f*temp_on_device[ty][tx]) * Ry_1 + 
          (temp_on_device[ty][E] + temp_on_device[ty][W] - 2.f*temp_on_device[ty][tx]) * Rx_1 + 
          (amb_temp - temp_on_device[ty][tx]) * Rz_1);

    }
    if(i==iteration-1)
      break;
    if(computed)   //Assign the computation range
      temp_on_device[ty][tx]= temp_t[ty][tx];
  }

  // update the global memory
  // after the last iteration, only threads coordinated within the 
  // small block perform the calculation and switch on ``computed''
  if (computed){
    temp_dst[index]= temp_t[ty][tx];    
  }
}

