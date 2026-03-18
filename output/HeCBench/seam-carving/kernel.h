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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <chrono>
#include <utility>  // std::swap

//#define STBI_ONLY_BMP
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define CUDA_CHECK(call)                                                    \
do {                                                                        \
    cudaError_t err_ = call;                                                \
} while (0)



// --- from kernels.h ---

//#define COMPUTE_COSTS_FULL
//#define COMPUTE_M_SINGLE
//#define COMPUTE_M_ITERATE

const int COSTS_BLOCKSIZE_X = 32;
const int COSTS_BLOCKSIZE_Y = 8;

const int COMPUTE_M_BLOCKSIZE_X = 128; //must be divisible by 2

const int REDUCE_BLOCKSIZE_X = 128;
const int REDUCE_ELEMENTS_PER_THREAD = 8;

const int REMOVE_BLOCKSIZE_X = 32;
const int REMOVE_BLOCKSIZE_Y = 8;

const int UPDATE_BLOCKSIZE_X = 32;
const int UPDATE_BLOCKSIZE_Y = 8;

const int APPROX_SETUP_BLOCKSIZE_X = 32;
const int APPROX_SETUP_BLOCKSIZE_Y = 8;

const int APPROX_M_BLOCKSIZE_X = 128;

pixel BORDER_PIXEL = {.r=0, .g=0, .b=0};

pixel pixel_from_uchar4(uchar4 uc4){
  pixel pix;
  pix.r = (int)uc4.x;
  pix.g = (int)uc4.y;
  pix.b = (int)uc4.z;
  return pix;
}   

void pointer_swap(void **p1, void **p2){
  void *tmp;
  tmp = *p1;
  *p1 = *p2;
  *p2 = tmp; 
}

void compute_costs_kernel(
    const uchar4 * d_pixels, 
    short * d_costs_left, 
    short * d_costs_up, 
    short * d_costs_right, 
    int w, int h, int current_w)
{
  //first row, first column and last column of shared memory are reserved for halo...
  pixel pix_cache[COSTS_BLOCKSIZE_Y][COSTS_BLOCKSIZE_X];
  //...and the global index in the image is computed accordingly to this 
  int row = _bid_y*(COSTS_BLOCKSIZE_Y-1) + _tid_y -1 ; 
  int column = _bid_x*(COSTS_BLOCKSIZE_X-2) + _tid_x -1; 
  int ix = row*w + column;
  int cache_row = _tid_y;
  int cache_column = _tid_x;
  short active = 0;

  if(row >= 0 && row < h && column >= 0 && column < current_w){
    active = 1;
    pix_cache[cache_row][cache_column] = pixel_from_uchar4(d_pixels[ix]);
  }
  else{
    pix_cache[cache_row][cache_column] = BORDER_PIXEL;
  }

  //wait until each thread has initialized its portion of shared memory

  //all the threads that are NOT in halo positions can now compute costs, with fast access to shared memory
  if(active && cache_row != 0 && cache_column != 0 && cache_column != COSTS_BLOCKSIZE_X-1){
    int rdiff, gdiff, bdiff;
    int p_r, p_g, p_b;
    pixel pix1, pix2, pix3;

    pix1 = pix_cache[cache_row][cache_column+1];
    pix2 = pix_cache[cache_row][cache_column-1];
    pix3 = pix_cache[cache_row-1][cache_column];

    //compute partials
    p_r = abs(pix1.r - pix2.r);
    p_g = abs(pix1.g - pix2.g);
    p_b = abs(pix1.b - pix2.b);

    //compute left cost       
    rdiff = p_r + abs(pix3.r - pix2.r);
    gdiff = p_g + abs(pix3.g - pix2.g);
    bdiff = p_b + abs(pix3.b - pix2.b);
    d_costs_left[ix] = rdiff + gdiff + bdiff;

    //compute up cost
    d_costs_up[ix] = p_r + p_g + p_b;

    //compute right cost
    rdiff = p_r + abs(pix3.r - pix1.r);
    gdiff = p_g + abs(pix3.g - pix1.g);
    bdiff = p_b + abs(pix3.b - pix1.b);
    d_costs_right[ix] = rdiff + gdiff + bdiff;         
  }
} 

void compute_costs_full_kernel(
    const uchar4*  d_pixels, 
    short * d_costs_left, 
    short * d_costs_up, 
    short * d_costs_right, 
    int w, int h, int current_w)
{
  pixel pix_cache[COSTS_BLOCKSIZE_Y+1][COSTS_BLOCKSIZE_X+2];
  int row = _bid_y*COSTS_BLOCKSIZE_Y + _tid_y; 
  int column = _bid_x*COSTS_BLOCKSIZE_X + _tid_x; 
  int ix = row*w + column;
  int cache_row = _tid_y + 1;
  int cache_column = _tid_x + 1;
  short active = 0;

  if(row < h && column < current_w){
    active = 1;
    if(_tid_x == 0){
      if(column == 0)
        pix_cache[cache_row][0] = BORDER_PIXEL;
      else
        pix_cache[cache_row][0] = pixel_from_uchar4(d_pixels[ix-1]);
    }
    if(_tid_x == COSTS_BLOCKSIZE_X-1 || column == current_w-1){
      if(column == current_w-1)
        pix_cache[cache_row][cache_column+1] = BORDER_PIXEL;
      else
        pix_cache[cache_row][COSTS_BLOCKSIZE_X+1] = pixel_from_uchar4(d_pixels[ix+1]);
    }
    if(_tid_y == 0){
      if(row == 0)
        pix_cache[0][cache_column] = BORDER_PIXEL;  
      else
        pix_cache[0][cache_column] = pixel_from_uchar4(d_pixels[ix-w]);          
    } 
    pix_cache[cache_row][cache_column] = pixel_from_uchar4(d_pixels[ix]);  
  }

  if(active){
    int rdiff, gdiff, bdiff;
    int p_r, p_g, p_b;
    pixel pix1, pix2, pix3;

    pix1 = pix_cache[cache_row][cache_column+1];
    pix2 = pix_cache[cache_row][cache_column-1];
    pix3 = pix_cache[cache_row-1][cache_column];

    //compute partials
    p_r = abs(pix1.r - pix2.r);
    p_g = abs(pix1.g - pix2.g);
    p_b = abs(pix1.b - pix2.b);

    //compute left cost       
    rdiff = p_r + abs(pix3.r - pix2.r);
    gdiff = p_g + abs(pix3.g - pix2.g);
    bdiff = p_b + abs(pix3.b - pix2.b);
    d_costs_left[ix] = rdiff + gdiff + bdiff;

    //compute up cost
    d_costs_up[ix] = p_r + p_g + p_b;

    //compute right cost
    rdiff = p_r + abs(pix3.r - pix1.r);
    gdiff = p_g + abs(pix3.g - pix1.g);
    bdiff = p_b + abs(pix3.b - pix1.b);
    d_costs_right[ix] = rdiff + gdiff + bdiff; 
  }
}

void compute_M_kernel_step1(
    const short * d_costs_left, 
    const short * d_costs_up, 
    const short * d_costs_right, 
    int*  d_M, 
    int w, int h, int current_w, int base_row)
{
  int cache[2*COMPUTE_M_BLOCKSIZE_X];
  int *m_cache = cache;
  int *m_cache_swap = &(cache[COMPUTE_M_BLOCKSIZE_X]);
  int column = _bid_x*COMPUTE_M_BLOCKSIZE_X + _tid_x; 
  int ix = base_row*w + column;
  int cache_column = _tid_x; 
  short is_first;
  short is_last;
  int right, up, left;

  is_first = _bid_x == 0;
  is_last = _bid_x == GRID_DIM_X-1;

  if(column < current_w){
    if(base_row == 0){
      left = min(d_costs_left[ix], min(d_costs_up[ix], d_costs_right[ix]));
      m_cache[cache_column] = left;
      d_M[ix] = left; 
    }
    else{
      m_cache[cache_column] = d_M[ix];    
    }
  }

  int max_row = base_row + COMPUTE_M_BLOCKSIZE_X/2;
  for(int row = base_row+1, inc = 1; row < max_row && row < h; row++, inc++){
    ix = ix + w;
    if(column < current_w && (is_first || inc <= _tid_x) && (is_last || _tid_x < COMPUTE_M_BLOCKSIZE_X - inc)){

      //with left
      if(column > 0)
        left = m_cache[cache_column - 1] + d_costs_left[ix]; 
      else 
        left = INT_MAX;
      //with up
      up = m_cache[cache_column] + d_costs_up[ix];
      //with right
      if(column < current_w-1)
        right = m_cache[cache_column + 1] + d_costs_right[ix];
      else
        right = INT_MAX;

      left = min(left, min(up, right));           
      d_M[ix] = left;
      //swap read/write shared memory
      pointer_swap((void**)&m_cache, (void**)&m_cache_swap);
      m_cache[cache_column] = left;
    }   
    //wait until every thread has written shared memory
  }
}

void compute_M_kernel_step2(
    const short * d_costs_left, 
    const short * d_costs_up, 
    const short * d_costs_right, 
    int*  d_M, 
    int w, int h, int current_w, int base_row)
{
  int column = _bid_x*COMPUTE_M_BLOCKSIZE_X + _tid_x + COMPUTE_M_BLOCKSIZE_X/2; 
  int right, up, left;

  int ix; 
  int prev_ix = base_row*w + column;
  int max_row = base_row + COMPUTE_M_BLOCKSIZE_X/2;
  for(int row = base_row+1, inc = 1; row < max_row && row < h; row++, inc++){
    ix = prev_ix + w;
    if(column < current_w && (COMPUTE_M_BLOCKSIZE_X/2 - inc <= _tid_x) && (_tid_x < COMPUTE_M_BLOCKSIZE_X/2 + inc)){
      //with left
      left = d_M[prev_ix - 1] + d_costs_left[ix]; 
      //with up
      up = d_M[prev_ix] + d_costs_up[ix];
      //with right
      if(column < current_w-1)
        right = d_M[prev_ix + 1] + d_costs_right[ix];
      else
        right = INT_MAX;

      left = min(left, min(up, right));               
      d_M[ix] = left;
    }
    prev_ix = ix;
  }
}

void compute_M_kernel_small(
    const short * d_costs_left, 
    const short * d_costs_up, 
    const short * d_costs_right, 
    int*  d_M, 
    int w, int h, int current_w)
{
  int cache[4096];
  int *m_cache = cache;
  int *m_cache_swap = &(cache[current_w]);
  int column = _tid_x;
  int ix = column;
  int left, up, right;

  //first row
  left = min(d_costs_left[ix], min(d_costs_up[ix], d_costs_right[ix]));
  d_M[ix] = left; 
  m_cache[ix] = left;

  //other rows
  for(int row = 1; row < h; row++){
    if(column < current_w){
      ix = ix + w;//ix = row*w + column;   

      //with left
      if(column > 0)
        left = m_cache[column - 1] + d_costs_left[ix]; 
      else
        left = INT_MAX;
      //with up
      up = m_cache[column] + d_costs_up[ix];
      //with right
      if(column < current_w-1)
        right = m_cache[column + 1] + d_costs_right[ix];
      else
        right = INT_MAX;

      left = min(left, min(up, right));            
      d_M[ix] = left;
      //swap read/write shared memory
      pointer_swap((void**)&m_cache, (void**)&m_cache_swap); 
      m_cache[column] = left;
    }
  }     
}

void compute_M_kernel_single(
    const short * d_costs_left, 
    const short * d_costs_up, 
    const short * d_costs_right, 
    int*  d_M, 
    int w, int h, int current_w, int n_elem)
{
  int cache[4096];
  int *m_cache = cache;
  int *m_cache_swap = &(cache[current_w]);
  int tid = _tid_x;
  int column; 
  int ix;
  int left, up, right;

  //first row
  for(int i = 0; i < n_elem; i++){
    column = tid + i*BLOCK_DIM_X;
    if(column < current_w){
      left = min(d_costs_left[column], min(d_costs_up[column], d_costs_right[column]));
      d_M[column] = left; 
      m_cache[column] = left;
    }
  }

  //other rows
  for(int row = 1; row < h; row++){
    for(int i = 0; i < n_elem; i++){
      column = tid + i*BLOCK_DIM_X;
      if(column < current_w){
        ix = row*w + column;

        //with left
        if(column > 0){
          left = m_cache[column - 1] + d_costs_left[ix]; 
        }
        else
          left = INT_MAX;
        //with up
        up = m_cache[column] + d_costs_up[ix];
        //with right
        if(column < current_w-1){
          right = m_cache[column + 1] + d_costs_right[ix];
        }
        else
          right = INT_MAX;

        left = min(left, min(up, right));
        d_M[ix] = left;
        m_cache_swap[column] = left;
      }          
    }    
    //swap read/write shared memory
    pointer_swap((void**)&m_cache, (void**)&m_cache_swap);
  }        
}

//compute M one row at a time with multiple kernel calls for global synchronization
void compute_M_kernel_iterate0(
    const short * d_costs_left, 
    const short * d_costs_up, 
    const short * d_costs_right, 
    int*  d_M, 
    int w, int current_w)
{
  int column = _bid_x*COMPUTE_M_BLOCKSIZE_X + _tid_x; 

  if(column < current_w){
    d_M[column] = min(d_costs_left[column], min(d_costs_up[column], d_costs_right[column]));
  }
}

void compute_M_kernel_iterate1(
    const short * d_costs_left, 
    const short * d_costs_up, 
    const short * d_costs_right, 
    int*  d_M, 
    int w, int current_w, int row)
{
  int column = _bid_x*COMPUTE_M_BLOCKSIZE_X + _tid_x; 
  int ix = row*w + column;
  int prev_ix = ix - w;
  int left, up, right;

  if(column < current_w){
    //with left
    if(column > 0)
      left = d_M[prev_ix - 1] + d_costs_left[ix]; 
    else
      left = INT_MAX;           
    //with up
    up = d_M[prev_ix] + d_costs_up[ix];        
    //with right
    if(column < current_w-1)
      right = d_M[prev_ix + 1] + d_costs_right[ix];
    else
      right = INT_MAX;

    d_M[ix] = min(left, min(up, right));  
  } 
}

void min_reduce(
    const int*  d_values,
    int*  d_indices,
    int size)
{
  int val_cache[REDUCE_BLOCKSIZE_X];
  int ix_cache[REDUCE_BLOCKSIZE_X];
  int tid = _tid_x;
  int column = _bid_x*REDUCE_BLOCKSIZE_X + tid;
  int grid_size = GRID_DIM_X*REDUCE_BLOCKSIZE_X;
  int min_v = INT_MAX;
  int min_i = 0;
  int new_i, new_v;

  for(int i = 0; i < REDUCE_ELEMENTS_PER_THREAD; i++){
    if(column < size){
      new_i = d_indices[column];
      new_v  = d_values[new_i];
      if(new_v < min_v){
        min_i = new_i;
        min_v = new_v;
      }
    } 
    column = column + grid_size;         
  }
  val_cache[tid] = min_v;
  ix_cache[tid] = min_i;

  for(int i = REDUCE_BLOCKSIZE_X/2; i > 0; i = i/2){
    if(tid < i){
      if(val_cache[tid + i] < val_cache[tid] || (val_cache[tid + i] == val_cache[tid] && ix_cache[tid + i] < ix_cache[tid])){
        val_cache[tid] = val_cache[tid + i];
        ix_cache[tid] = ix_cache[tid + i];
      }
    }
  }

  if(tid == 0){
    d_indices[_bid_x] = ix_cache[0];  
  }  
}

void find_seam_kernel(
    const int * d_M,
    const int * d_indices,
    int * d_seam,
    int w, int h, int current_w)
{
  int base_row, mid;
  int min_index = d_indices[0];

  d_seam[h-1] = min_index; 
  for(int row = h-2; row >= 0; row--){
    base_row = row*w;
    mid = min_index;
    if(mid != 0){
      if(d_M[base_row + mid - 1] < d_M[base_row + min_index])
        min_index = mid - 1;
    }
    if(mid != current_w){
      if(d_M[base_row + mid + 1] < d_M[base_row + min_index])
        min_index = mid + 1;
    }
    d_seam[row] = min_index;
  }
}

void remove_seam_kernel(
    const uchar4 * d_pixels, 
          uchar4 * d_pixels_swap, 
    const int * d_seam, 
    int w, int h, int current_w)
{
  int row = _bid_y*REMOVE_BLOCKSIZE_Y + _tid_y;
  int column = _bid_x*REMOVE_BLOCKSIZE_X + _tid_x;

  if(row < h && column < current_w-1){
    int seam_c = d_seam[row];
    int ix = row*w + column;
    d_pixels_swap[ix] = (column >= seam_c) ? d_pixels[ix + 1] : d_pixels[ix];
  }
}

void update_costs_kernel(
    const uchar4 * d_pixels, 
    const short * d_costs_left, 
    const short * d_costs_up, 
    const short * d_costs_right, 
    short * d_costs_swap_left, 
    short * d_costs_swap_up, 
    short * d_costs_swap_right, 
    const int * d_seam, 
    int w, int h, int current_w)
{
  int row = _bid_y*UPDATE_BLOCKSIZE_Y + _tid_y;
  int column = _bid_x*UPDATE_BLOCKSIZE_X + _tid_x;

  if(row < h && column < current_w-1){
    int seam_c = d_seam[row];
    int ix = row*w + column;
    if(column >= seam_c-2 && column <= seam_c+1){
      //update costs near removed seam
      pixel pix1, pix2, pix3;
      int p_r, p_g, p_b;
      int rdiff, gdiff, bdiff;          

      if(column == current_w-2) 
        pix1 = BORDER_PIXEL;
      else
        pix1 = pixel_from_uchar4(d_pixels[ix + 1]);
      if(column == 0)
        pix2 = BORDER_PIXEL;
      else
        pix2 = pixel_from_uchar4(d_pixels[ix - 1]);
      if(row == 0)
        pix3 = BORDER_PIXEL;
      else
        pix3 = pixel_from_uchar4(d_pixels[ix - w]);

      //compute partials
      p_r = abs(pix1.r - pix2.r);
      p_g = abs(pix1.g - pix2.g);
      p_b = abs(pix1.b - pix2.b);

      //compute left cost       
      rdiff = p_r + abs(pix3.r - pix2.r);
      gdiff = p_g + abs(pix3.g - pix2.g);
      bdiff = p_b + abs(pix3.b - pix2.b);
      d_costs_swap_left[ix] = rdiff + gdiff + bdiff;

      //compute up cost
      d_costs_swap_up[ix] = p_r + p_g + p_b;

      //compute right cost
      rdiff = p_r + abs(pix3.r - pix1.r);
      gdiff = p_g + abs(pix3.g - pix1.g);
      bdiff = p_b + abs(pix3.b - pix1.b);
      d_costs_swap_right[ix] = rdiff + gdiff + bdiff;             
    }
    else if(column > seam_c+1){
      //shift costs to the left
      d_costs_swap_left[ix] = d_costs_left[ix + 1];
      d_costs_swap_up[ix] = d_costs_up[ix + 1];
      d_costs_swap_right[ix] = d_costs_right[ix + 1];
    }
    else{
      //copy remaining costs
      d_costs_swap_left[ix] = d_costs_left[ix];
      d_costs_swap_up[ix] = d_costs_up[ix];
      d_costs_swap_right[ix] = d_costs_right[ix];
    }
  }
}

void approx_setup_kernel(
    const uchar4 * d_pixels, 
    int * d_index_map, 
    int * d_offset_map, 
    int * d_M, int w, int h, int current_w)
{
  pixel pix_cache[APPROX_SETUP_BLOCKSIZE_Y][APPROX_SETUP_BLOCKSIZE_X];
  short left_cache[APPROX_SETUP_BLOCKSIZE_Y][APPROX_SETUP_BLOCKSIZE_X];
  short up_cache[APPROX_SETUP_BLOCKSIZE_Y][APPROX_SETUP_BLOCKSIZE_X];
  short right_cache[APPROX_SETUP_BLOCKSIZE_Y][APPROX_SETUP_BLOCKSIZE_X];
  int row = _bid_y*(APPROX_SETUP_BLOCKSIZE_Y-1) + _tid_y -1 ; 
  int column = _bid_x*(APPROX_SETUP_BLOCKSIZE_X-4) + _tid_x -2; //WE NEED MORE HORIZONTAL HALO...
  int ix = row*w + column;
  int cache_row = _tid_y;
  int cache_column = _tid_x;
  short active = 0;

  if(row >= 0 && row < h && column >= 0 && column < current_w){
    active = 1;
    pix_cache[cache_row][cache_column] = pixel_from_uchar4(d_pixels[ix]);
  }
  else{
    pix_cache[cache_row][cache_column] = BORDER_PIXEL;
  }

  //wait until each thread has initialized its portion of shared memory

  if(active && cache_row > 0){
    int rdiff, gdiff, bdiff;
    int p_r, p_g, p_b;
    pixel pix1, pix2, pix3;

    if(cache_column < APPROX_SETUP_BLOCKSIZE_X-1){
      pix1 = pix_cache[cache_row][cache_column+1];   //...OR ELSE WE CANNOT CALCULATE LEFT COST FOR THE LAST THREAD IN THE BLOCK (pix1 dependance)
    }

    if(cache_column > 0){
      pix2 = pix_cache[cache_row][cache_column-1];   //SAME THING WITH RIGHT COST FOR THE FIRST THREAD (pix2 dependance)
    }

    pix3 = pix_cache[cache_row-1][cache_column];

    //compute partials
    p_r = abs(pix1.r - pix2.r);
    p_g = abs(pix1.g - pix2.g);
    p_b = abs(pix1.b - pix2.b);

    //compute left cost       
    rdiff = p_r + abs(pix3.r - pix2.r);
    gdiff = p_g + abs(pix3.g - pix2.g);
    bdiff = p_b + abs(pix3.b - pix2.b);
    left_cache[cache_row][cache_column] = rdiff + gdiff + bdiff;

    //compute up cost
    up_cache[cache_row][cache_column] = p_r + p_g + p_b;

    //compute right cost
    rdiff = p_r + abs(pix3.r - pix1.r);
    gdiff = p_g + abs(pix3.g - pix1.g);
    bdiff = p_b + abs(pix3.b - pix1.b);
    right_cache[cache_row][cache_column] = rdiff + gdiff + bdiff;             
  }

  if(active && row < h-1 && cache_column > 1 && cache_column < APPROX_SETUP_BLOCKSIZE_X-2 && cache_row != APPROX_SETUP_BLOCKSIZE_Y-1){
    int min_cost = INT_MAX;
    int map_ix;
    int cost;

    if(column > 0){
      min_cost = right_cache[cache_row+1][cache_column-1];
      map_ix = ix + w - 1;
    }

    cost = up_cache[cache_row+1][cache_column];
    if(cost < min_cost){
      min_cost = cost;
      map_ix = ix + w;
    }

    if(column < current_w-1){
      cost = left_cache[cache_row+1][cache_column+1];
      if(cost < min_cost){
        min_cost = cost;
        map_ix = ix + w + 1;
      }
    }

    d_index_map[ix] = map_ix;
    d_offset_map[ix] = map_ix;
    d_M[ix] = min_cost;           
  }
} 

void approx_M_kernel(
    int * d_offset_map,
    int * d_M,
    int w, int h, int current_w, int step)
{
  int row = _bid_y*2*step;
  int next_row = row + step;
  int column = _bid_x*APPROX_M_BLOCKSIZE_X + _tid_x;
  int ix = row*w + column;

  if(next_row < h-1 && column < current_w){
    int offset = d_offset_map[ix];
    d_M[ix] += d_M[offset];
    d_offset_map[ix] = d_offset_map[offset];
  }
}

void approx_seam_kernel(
    const int * d_index_map, 
    const int * d_indices, 
    int * d_seam, 
    int w, int h)
{
  int ix;
  ix = d_indices[0]; //min index
  for(int i = 0; i < h; i++){
    d_seam[i] = ix - i*w;
    ix = d_index_map[ix];
  }
}



// --- from kernels_wrapper.h ---
/* ################# wrappers ################### */

void compute_costs(
    int current_w, int w, int h, 
    uchar4* d_pixels, 
    short* d_costs_left,
    short* d_costs_up,
    short* d_costs_right)
{
#ifndef COMPUTE_COSTS_FULL

  dim3 threads_per_block(COSTS_BLOCKSIZE_X, COSTS_BLOCKSIZE_Y);
  dim3 num_blocks;
  num_blocks.x = (current_w-1)/(threads_per_block.x-2) + 1;
  num_blocks.y = (h-1)/(threads_per_block.y-1) + 1;    

      w, h, current_w);

#else

  dim3 threads_per_block(COSTS_BLOCKSIZE_X, COSTS_BLOCKSIZE_Y);
  dim3 num_blocks;
  num_blocks.x = (current_w-1)/threads_per_block.x + 1;
  num_blocks.y = (h-1)/threads_per_block.y + 1;    

      w, h, current_w);

#endif
}

void compute_M(
    int current_w, int w, int h, 
    int* d_M, 
    short* d_costs_left,
    short* d_costs_up,
    short* d_costs_right)
{
#if !defined(COMPUTE_M_SINGLE) && !defined(COMPUTE_M_ITERATE)  

  if(current_w <= 256){
    dim3 threads_per_block(current_w);
    dim3 num_blocks(1);

  }
  else{
    dim3 threads_per_block(COMPUTE_M_BLOCKSIZE_X, 1);

    dim3 num_blocks;
    num_blocks.x = (current_w-1)/threads_per_block.x + 1;
    num_blocks.y = 1;

    dim3 num_blocks2;
    num_blocks2.x = (current_w-COMPUTE_M_BLOCKSIZE_X-1)/threads_per_block.x + 1; 
    num_blocks2.y = 1;  

    int num_iterations;
    num_iterations = (h-1)/(COMPUTE_M_BLOCKSIZE_X/2 - 1) + 1;

    int base_row = 0;
    for(int i = 0; i < num_iterations; i++){

      base_row = base_row + (COMPUTE_M_BLOCKSIZE_X/2) - 1;    
    }
  }

#endif
#ifdef COMPUTE_M_SINGLE    

  dim3 threads_per_block(min(256, next_pow2(current_w)), 1);   
  dim3 num_blocks(1,1);
  int num_el = (current_w-1)/threads_per_block.x + 1;

#else
#ifdef COMPUTE_M_ITERATE

  dim3 threads_per_block(COMPUTE_M_BLOCKSIZE_X, 1);   
  dim3 num_blocks;
  num_blocks.x = (current_w-1)/threads_per_block.x + 1;
  num_blocks.y = 1;

  for(int row = 1; row < h; row++){

  }

#endif
#endif
}

void find_min_index(
    int current_w,
    int* d_indices_ref,
    int* d_indices,
    int* reduce_row)
{
  //set the reference index array

  dim3 threads_per_block(REDUCE_BLOCKSIZE_X, 1);   
  dim3 num_blocks;
  num_blocks.y = 1; 
  int reduce_num_elements = current_w;
  do{
    num_blocks.x = (reduce_num_elements-1)/(threads_per_block.x*REDUCE_ELEMENTS_PER_THREAD) + 1;

    reduce_num_elements = num_blocks.x;          
  }while(num_blocks.x > 1);    
}

void find_seam(
    int current_w, int w, int h, 
    int *d_M,
    int *d_indices,
    int *d_seam )
{

}

void remove_seam(
    int current_w, int w, int h, 
    int *d_M,
    uchar4 *d_pixels,
    uchar4 *d_pixels_swap,
    int *d_seam )
{
  dim3 threads_per_block(REMOVE_BLOCKSIZE_X, REMOVE_BLOCKSIZE_Y);
  dim3 num_blocks;
  num_blocks.x = (current_w-1)/threads_per_block.x + 1;
  num_blocks.y = (h-1)/threads_per_block.y + 1;

}

void update_costs(
    int current_w, int w, int h, 
    int *d_M,
    uchar4 *d_pixels,
    short *d_costs_left,
    short *d_costs_up,
    short *d_costs_right,
    short *d_costs_swap_left,
    short *d_costs_swap_up,
    short *d_costs_swap_right,
    int *d_seam )
{
  dim3 threads_per_block(UPDATE_BLOCKSIZE_X, UPDATE_BLOCKSIZE_Y);
  dim3 num_blocks;
  num_blocks.x = (current_w-1)/threads_per_block.x + 1;
  num_blocks.y = (h-1)/threads_per_block.y + 1;    

    d_pixels, d_costs_left, d_costs_up, d_costs_right, 
    d_costs_swap_left, d_costs_swap_up, d_costs_swap_right,
    d_seam, w, h, current_w);
}

void approx_setup(
    int current_w, int w, int h, 
    uchar4 *d_pixels,
    int *d_index_map,
    int *d_offset_map,
    int *d_M )
{
  dim3 threads_per_block(APPROX_SETUP_BLOCKSIZE_X, APPROX_SETUP_BLOCKSIZE_Y);
  dim3 num_blocks;
  num_blocks.x = (current_w-1)/(threads_per_block.x-4) + 1;
  num_blocks.y = (h-2)/(threads_per_block.y-1) + 1;    

}

void approx_M(
    int current_w, int w, int h, 
    int *d_offset_map,
    int *d_M )
{
  dim3 threads_per_block(APPROX_M_BLOCKSIZE_X, 1);
  dim3 num_blocks;
  num_blocks.x = (current_w-1)/threads_per_block.x + 1;
  num_blocks.y = h/2;  
  int step = 1;
  while(num_blocks.y > 0){

    num_blocks.y = (int)num_blocks.y/2;
    step = step*2;
  }
}

void approx_seam(
    int w, int h, 
    int *d_index_map,
    int *d_indices,
    int *d_seam )
{

}


// --- from utils.h ---
typedef enum {
    SEAM_CARVER_STANDARD_MODE,
    SEAM_CARVER_UPDATE_MODE,
    SEAM_CARVER_APPROX_MODE
} seam_carver_mode;

typedef struct { int r; int g; int b; } pixel;

int next_pow2(int n){
  int res = 1;
  while(res < n)
    res = res*2;
  return res;
}

uchar4 *build_pixels(const unsigned char *imgv, int w, int h){
  uchar4 *pixels = (uchar4*)malloc((size_t)w*h*sizeof(uchar4));
  uchar4 pix;
  for(int i = 0; i < h; i++){
    for(int j = 0; j < w; j++){
      pix.x = imgv[i*3*w + 3*j];
      pix.y = imgv[i*3*w + 3*j + 1];
      pix.z = imgv[i*3*w + 3*j + 2]; 
      pixels[i*w + j] = pix;            
    }
  }
  return pixels;
}

unsigned char *flatten_pixels(uchar4 *pixels, int w, int h, int new_w){
  unsigned char *flattened = (unsigned char*)malloc(3*new_w*h*sizeof(unsigned char));
  for(int i = 0; i < h; i++){
    for(int j = 0; j < new_w; j++){ 
      uchar4 pix = pixels[i*w + j];
      flattened[3*i*new_w + 3*j] = pix.x;
      flattened[3*i*new_w + 3*j + 1] = pix.y;
      flattened[3*i*new_w + 3*j + 2] = pix.z;
    }
  }
  return flattened;
}

