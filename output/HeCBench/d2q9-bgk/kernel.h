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
/*
 ** Code to implement a d2q9-bgk lattice boltzmann scheme.
 ** 'd2' inidates a 2-dimensional grid, and
 ** 'q9' indicates 9 velocities per grid cell.
 ** 'bgk' refers to the Bhatnagar-Gross-Krook collision step.
 **
 ** The 'speeds' in each cell are numbered as follows:
 **
 ** 6 2 5
 **  \|/
 ** 3-0-1
 **  /|\
 ** 7 4 8
 **
 ** A 2D grid:
 **
 **           cols
 **       --- --- ---
 **      | D | E | F |
 ** rows  --- --- ---
 **      | A | B | C |
 **       --- --- ---
 **
 ** 'unwrapped' in row major order to give a 1D array:
 **
 **  --- --- --- --- --- ---
 ** | A | B | C | D | E | F |
 **  --- --- --- --- --- ---
 **
 ** Grid indicies are:
 **
 **          ny
 **          ^       cols(ii)
 **          |  ----- ----- -----
 **          | | ... | ... | etc |
 **          |  ----- ----- -----
 ** rows(jj) | | 1,0 | 1,1 | 1,2 |
 **          |  ----- ----- -----
 **          | | 0,0 | 0,1 | 0,2 |
 **          |  ----- ----- -----
 **          ----------------------> nx
 **
 ** Note the names of the input parameter and obstacle files
 ** are passed on the command line, e.g.:
 **
 **   ./d2q9-bgk input.params obstacles.dat
 **
 ** Be sure to adjust the grid dimensions in the parameter file
 ** if you choose a different obstacle file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>

#define WARMUPS         1000
#define NSPEEDS         9
#define LOCALSIZEX      128
#define LOCALSIZEY      1

/* dump output files for verification */
#define FINALSTATEFILE  "final_state.dat"
#define AVVELSFILE      "av_vels.dat"

/* struct to hold the parameter values */
typedef struct
{
  int   nx;            /* no. of cells in x-direction */
  int   ny;            /* no. of cells in y-direction */
  int   maxIters;      /* no. of iterations */
  int   reynolds_dim;  /* dimension for Reynolds number */
  float density;       /* density per link */
  float accel;         /* density redistribution */
  float omega;         /* relaxation parameter */
} t_param;

/* struct to hold the 'speed' values */
typedef struct
{
  float speeds[NSPEEDS];
} t_speed;

/*
 ** function prototypes
 */

/* load params, allocate memory, load obstacles & initialise fluid particle densities */
int initialise(const char* paramfile, const char* obstaclefile,
    t_param* params, t_speed** cells_ptr, t_speed** tmp_cells_ptr,
    int** obstacles_ptr, float** av_vels_ptr);

/*
 ** The main calculation methods.
 ** timestep calls, in order, the functions:
 ** accelerate_flow(), propagate(), rebound() & collision()
 */
int write_values(const t_param params, t_speed* cells, int* obstacles, float* av_vels);

/* finalise, including freeing up allocated memory */
int finalise(t_speed* cells_ptr, t_speed* tmp_cells_ptr,
    int* obstacles_ptr, float* av_vels_ptr);

/* Sum all the densities in the grid.
 ** The total should remain constant from one timestep to the next. */
float total_density(const t_param params, t_speed* cells);

/* compute average velocity */
float av_velocity(const t_param params, t_speed* cells, int* obstacles);

/* calculate Reynolds number */
float calc_reynolds(const t_param params, t_speed* cells, int* obstacles);

/* utility functions */
void die(const char* message, const int line, const char* file);

void d2q9_bgk(
  const float*  Speed0A,
  const float*  Speed1A,
  const float*  Speed2A,
  const float*  Speed3A,
  const float*  Speed4A,
  const float*  Speed5A,
  const float*  Speed6A,
  const float*  Speed7A,
  const float*  Speed8A,
  float*  Tmp0A,
  float*  Tmp1A,
  float*  Tmp2A,
  float*  Tmp3A,
  float*  Tmp4A,
  float*  Tmp5A,
  float*  Tmp6A,
  float*  Tmp7A,
  float*  Tmp8A,
  const int*  ObstaclesA,
  float*  Partial_Sum,
  int*  Partial_Sum2,
  const float densityaccel,
  const float omega,
  const int nx,
  const int ny,
  const int tt)
{
  //setup local memory
  int local_sum2[LOCALSIZEX*LOCALSIZEY];
  float local_sum[LOCALSIZEX*LOCALSIZEY];

  /* get column and row indices */
  const int ii = _bid_x * BLOCK_DIM_X + _tid_x;
  const int jj = _bid_y * BLOCK_DIM_Y + _tid_y;

  const float c_sq_inv = 3.f;
  const float c_sq = 1.f/c_sq_inv; /* square of speed of sound */
  const float temp1 = 4.5f;
  const float w1 = 1.f/9.f;
  const float w0 = 4.f * w1;  /* weighting factor */
  const float w2 = 1.f/36.f; /* weighting factor */
  const float w11 = densityaccel * w1;
  const float w21 = densityaccel * w2;

  /* determine indices of axis-direction neighbours
   ** respecting periodic boundary conditions (wrap around) */
  const int y_n = (jj + 1) % ny;
  const int x_e = (ii + 1) % nx;
  const int y_s = (jj == 0) ? (jj + ny - 1) : (jj - 1);
  const int x_w = (ii == 0) ? (ii + nx - 1) : (ii - 1);

  /* propagate densities from neighbouring cells, following
   ** appropriate directions of travel and writing into
   ** scratch space grid */

  float tmp_s0 = Speed0A[ii + jj*nx];
  float tmp_s1 = (jj == ny-2 && (!ObstaclesA[x_w + jj*nx] && isGreater((Speed3A[x_w + jj*nx] - w11) , 0.f) && isGreater((Speed6A[x_w + jj*nx] - w21) , 0.f) && isGreater((Speed7A[x_w + jj*nx] - w21) , 0.f))) ? Speed1A[x_w + jj*nx]+w11 : Speed1A[x_w + jj*nx];
  float tmp_s2 = Speed2A[ii + y_s*nx];
  float tmp_s3 = (jj == ny-2 && (!ObstaclesA[x_e + jj*nx] && isGreater((Speed3A[x_e + jj*nx] - w11) , 0.f) && isGreater((Speed6A[x_e + jj*nx] - w21) , 0.f) && isGreater((Speed7A[x_e + jj*nx] - w21) , 0.f))) ? Speed3A[x_e + jj*nx]-w11 : Speed3A[x_e + jj*nx];
  float tmp_s4 = Speed4A[ii + y_n*nx];
  float tmp_s5 = (y_s == ny-2 && (!ObstaclesA[x_w + y_s*nx] && isGreater((Speed3A[x_w + y_s*nx] - w11) , 0.f) && isGreater((Speed6A[x_w + y_s*nx] - w21) , 0.f) && isGreater((Speed7A[x_w + y_s*nx] - w21) , 0.f))) ? Speed5A[x_w + y_s*nx]+w21 : Speed5A[x_w + y_s*nx];
  float tmp_s6 = (y_s == ny-2 && (!ObstaclesA[x_e + y_s*nx] && isGreater((Speed3A[x_e + y_s*nx] - w11) , 0.f) && isGreater((Speed6A[x_e + y_s*nx] - w21) , 0.f) && isGreater((Speed7A[x_e + y_s*nx] - w21) , 0.f))) ? Speed6A[x_e + y_s*nx]-w21 : Speed6A[x_e + y_s*nx];
  float tmp_s7 = (y_n == ny-2 && (!ObstaclesA[x_e + y_n*nx] && isGreater((Speed3A[x_e + y_n*nx] - w11) , 0.f) && isGreater((Speed6A[x_e + y_n*nx] - w21) , 0.f) && isGreater((Speed7A[x_e + y_n*nx] - w21) , 0.f))) ? Speed7A[x_e + y_n*nx]-w21 : Speed7A[x_e + y_n*nx];
  float tmp_s8 = (y_n == ny-2 && (!ObstaclesA[x_w + y_n*nx] && isGreater((Speed3A[x_w + y_n*nx] - w11) , 0.f) && isGreater((Speed6A[x_w + y_n*nx] - w21) , 0.f) && isGreater((Speed7A[x_w + y_n*nx] - w21) , 0.f))) ? Speed8A[x_w + y_n*nx]+w21 : Speed8A[x_w + y_n*nx];

  /* compute local density total */
  float local_density = tmp_s0 + tmp_s1 + tmp_s2 + tmp_s3 + tmp_s4  + tmp_s5  + tmp_s6  + tmp_s7  + tmp_s8;
  const float local_density_recip = 1.f/(local_density);
  /* compute x velocity component */
  float u_x = (tmp_s1
      + tmp_s5
      + tmp_s8
      - tmp_s3
      - tmp_s6
      - tmp_s7)
    * local_density_recip;
  /* compute y velocity component */
  float u_y = (tmp_s2
      + tmp_s5
      + tmp_s6
      - tmp_s4
      - tmp_s8
      - tmp_s7)
    * local_density_recip;

  /* velocity squared */
  const float temp2 = - (u_x * u_x + u_y * u_y)/(2.f * c_sq);

  /* equilibrium densities */
  float d_equ[NSPEEDS];
  /* zero velocity density: weight w0 */
  d_equ[0] = w0 * local_density
    * (1.f + temp2);
  /* axis speeds: weight w1 */
  d_equ[1] = w1 * local_density * (1.f + u_x * c_sq_inv
      + (u_x * u_x) * temp1
      + temp2);
  d_equ[2] = w1 * local_density * (1.f + u_y * c_sq_inv
      + (u_y * u_y) * temp1
      + temp2);
  d_equ[3] = w1 * local_density * (1.f - u_x * c_sq_inv
      + (u_x * u_x) * temp1
      + temp2);
  d_equ[4] = w1 * local_density * (1.f - u_y * c_sq_inv
      + (u_y * u_y) * temp1
      + temp2);
  /* diagonal speeds: weight w2 */
  d_equ[5] = w2 * local_density * (1.f + (u_x + u_y) * c_sq_inv
      + ((u_x + u_y) * (u_x + u_y)) * temp1
      + temp2);
  d_equ[6] = w2 * local_density * (1.f + (-u_x + u_y) * c_sq_inv
      + ((-u_x + u_y) * (-u_x + u_y)) * temp1
      + temp2);
  d_equ[7] = w2 * local_density * (1.f + (-u_x - u_y) * c_sq_inv
      + ((-u_x - u_y) * (-u_x - u_y)) * temp1
      + temp2);
  d_equ[8] = w2 * local_density * (1.f + (u_x - u_y) * c_sq_inv
      + ((u_x - u_y) * (u_x - u_y)) * temp1
      + temp2);

  float tmp;
  int expression = ObstaclesA[ii + jj*nx];
  tmp_s0 = expression ? tmp_s0 : (tmp_s0 + omega * (d_equ[0] - tmp_s0));
  tmp = tmp_s1;
  tmp_s1 = expression ? tmp_s3 : (tmp_s1 + omega * (d_equ[1] - tmp_s1));
  tmp_s3 = expression ? tmp : (tmp_s3 + omega * (d_equ[3] - tmp_s3));
  tmp = tmp_s2;
  tmp_s2 = expression ? tmp_s4 : (tmp_s2 + omega * (d_equ[2] - tmp_s2));
  tmp_s4 = expression ? tmp : (tmp_s4 + omega * (d_equ[4] - tmp_s4));
  tmp = tmp_s5;
  tmp_s5 = expression ? tmp_s7 : (tmp_s5 + omega * (d_equ[5] - tmp_s5));
  tmp_s7 = expression ? tmp : (tmp_s7 + omega * (d_equ[7] - tmp_s7));
  tmp = tmp_s6;
  tmp_s6 = expression ? tmp_s8 : (tmp_s6 + omega * (d_equ[6] - tmp_s6));
  tmp_s8 = expression ? tmp : (tmp_s8 + omega * (d_equ[8] - tmp_s8));

  /* local density total */
  local_density = 1.f/(tmp_s0 + tmp_s1 + tmp_s2 + tmp_s3 + tmp_s4 + tmp_s5 + tmp_s6 + tmp_s7 + tmp_s8);

  /* x-component of velocity */
  u_x = (tmp_s1
      + tmp_s5
      + tmp_s8
      - tmp_s3
      - tmp_s6
      - tmp_s7)
    * local_density;
  /* compute y velocity component */
  u_y = (tmp_s2
      + tmp_s5
      + tmp_s6
      - tmp_s4
      - tmp_s7
      - tmp_s8)
    * local_density;

  Tmp0A[ii + jj*nx] = tmp_s0;
  Tmp1A[ii + jj*nx] = tmp_s1;
  Tmp2A[ii + jj*nx] = tmp_s2;
  Tmp3A[ii + jj*nx] = tmp_s3;
  Tmp4A[ii + jj*nx] = tmp_s4;
  Tmp5A[ii + jj*nx] = tmp_s5;
  Tmp6A[ii + jj*nx] = tmp_s6;
  Tmp7A[ii + jj*nx] = tmp_s7;
  Tmp8A[ii + jj*nx] = tmp_s8;

  int local_idi = _tid_x;
  int local_idj = _tid_y;
  int local_sizei = BLOCK_DIM_X;
  int local_sizej = BLOCK_DIM_Y;

  /* accumulate the norm of x- and y- velocity components */
  local_sum[local_idi + local_idj*local_sizei] = (ObstaclesA[ii + jj*nx]) ? 0 : hypotf(u_x,u_y);
  /* increase counter of inspected cells */
  local_sum2[local_idi + local_idj*local_sizei] = (ObstaclesA[ii + jj*nx]) ? 0 : 1 ;

  int group_id = _bid_x;
  int group_id2 = _bid_y; 
  int group_size = GRID_DIM_X;
  int group_size2 = GRID_DIM_Y;
}









