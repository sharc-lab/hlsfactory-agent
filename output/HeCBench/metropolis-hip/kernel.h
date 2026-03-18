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
#ifndef BLOCK_DIM_Z
#define BLOCK_DIM_Z 1
#endif
#ifndef GRID_DIM_Z
#define GRID_DIM_Z 1
#endif

// --- from heap.cu ---
/*
    File:   minHeap.c
    Desc:   Program showing various operations on a binary min heap
    Author: Robin Thomas <robinthomas2591@gmail.com>
*/

#include <stdio.h>
#include <stdlib.h>
/*
    Function to initialize the min heap with size = 0
*/

/*
    Function to swap data within two nodes of the min heap using pointers
*/


/*
    Heapify function is used to make sure that the heap property is never violated
    In case of deletion of a node, or creating a min heap from an array, heap property
    may be violated. In such cases, heapify function can be called to make sure that
    heap property is never violated
*/

/* 
    Build a Min Heap given an array of numbers

/*
    Function to insert a node into the min heap, by allocating space for that node in the
    heap and also making sure that the heap property and shape propety are never violated.
*/

/*
    Function to delete a node from the min heap
    It shall remove the root node, and place the last node in its place
    and then call heapify function to make sure that the heap property
    is never violated
*/

/*
    Function to pop the min value form the root and heapify accordingly
*/

/*
    Function to get maximum node from a min heap
    The maximum node shall always be one of the leaf nodes. So we shall recursively
    move through both left and right child, until we find their maximum nodes, and
    compare which is larger. It shall be done recursively until we get the maximum
    node
*/

/*
    Function to clear the memory allocated for the min heap
*/

/*
    Function to display all the nodes in the min heap by doing a inorder traversal
*/

/*
    Function to display all the nodes in the min heap by doing a preorder traversal
*/

/*
    Function to display all the nodes in the min heap by doing a post order traversal
*/

/*
    Function to display all the nodes in the min heap by doing a level order traversal
*/



// --- from main.cu ---
//////////////////////////////////////////////////////////////////////////////////
//                                                                              //
//  trueke                                                                      //
//  A multi-GPU implementation of the exchange Monte Carlo method.              //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////
//                                                                              //
//  Copyright © 2015 Cristobal A. Navarro, Wei Huang.                           //
//                                                                              //
//  This file is part of trueke.                                                //
//  trueke is free software: you can redistribute it and/or modify              //
//  it under the terms of the GNU General Public License as published by        //
//  the Free Software Foundation, either version 3 of the License, or           //
//  (at your option) any later version.                                         //
//                                                                              //
//  trueke is distributed in the hope that it will be useful,                   //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of              //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               //
//  GNU General Public License for more details.                                //
//                                                                              //
//  You should have received a copy of the GNU General Public License           //
//  along with trueke.  If not, see <http://www.gnu.org/licenses/>.             //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////



// --- from utils.cu ---

/* compare two floats */


/* get the left frag position */

/* put the new value well distributed in a GPU */

/* rebuild the temperatures, sorted */

/* insert temperatures at the "ins" lowest exchange places */

/* rebuild atrs and arts indices */


/* print indexed fragmented array */

/* print indexed fragmented array */






// --- from findex.h ---
#ifndef FINDEX_H
#define FINDEX_H

/* fragmented index structure */
struct findex{ int f; int i; };
typedef findex findex_t;

#endif


// --- from heap.h ---
#ifndef HEAP_H
#define HEAP_H

#define LCHILD(x) 2 * x + 1
#define RCHILD(x) 2 * x + 2
#define PARENT(x) x / 2


/* node struct */
typedef struct node {
    float data ;
    findex_t coord;
} node ;

/* min heap */
typedef struct minHeap {
    int size ;
    node *elem ;
} minHeap ;

minHeap initMinHeap(int size);
void swap(node *n1, node *n2);
void printNode(node n);
void heapify(minHeap *hp, int i);
void buildMinHeap(minHeap *hp, int *arr, int size);
void insertNode(minHeap *hp, float data, findex_t frag);
void deleteNode(minHeap *hp);
node popRoot(minHeap *hp);
int getMaxNode(minHeap *hp, int i);
void deleteMinHeap(minHeap *hp);
void inorderTraversal(minHeap *hp, int i);
void preorderTraversal(minHeap *hp, int i);
void postorderTraversal(minHeap *hp, int i);
void levelorderTraversal(minHeap *hp);

#endif


// --- from kernel_metropolis.h ---
//////////////////////////////////////////////////////////////////////////////////
//                                                                              //
//  trueke                                                                      //
//  A multi-GPU implementation of the exchange Monte Carlo method.              //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////
//                                                                              //
//  Copyright © 2015 Cristobal A. Navarro, Wei Huang.                           //
//                                                                              //
//  This file is part of trueke.                                                //
//  trueke is free software: you can redistribute it and/or modify              //
//  it under the terms of the GNU General Public License as published by        //
//  the Free Software Foundation, either version 3 of the License, or           //
//  (at your option) any later version.                                         //
//                                                                              //
//  trueke is distributed in the hope that it will be useful,                   //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of              //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               //
//  GNU General Public License for more details.                                //
//                                                                              //
//  You should have received a copy of the GNU General Public License           //
//  along with trueke.  If not, see <http://www.gnu.org/licenses/>.             //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////

#ifndef _KERNEL_MONTECARLO_CUH_
#define _KERNEL_MONTECARLO_CUH_

// GPU-related definitions

#define sLx (BX+2)
#define sLy (BY+2)
#define sLz (BZ+2)

#define SVOLUME ((sLx)*(sLy)*(sLz))
#define BVOLUME ((BX)*((BY)/2)*(BZ))

#define BLOCKSIZE 8
#define BLOCK_STEPS 1
#define EPSILON 0.00000000001f

#define C(x,y,z,L)     ((z)*(L)*(L)+(y)*(L)+(x))
#define sC(x,y,z,Lx,Ly)  ((z+1)*(Ly)*(Lx)+(y+1)*(Lx)+(x+1))

typedef int site_t;
// kernel_metropolis
void
kernel_metropolis(const int N, const int L, site_t *s, const int *H, 
                  const float h, const float B, uint64_t *state, uint64_t *inc, int alt)
{

  // offsets
  int offx = _bid_x * BX;
  int offy = (2*_bid_y + ((_bid_x + _bid_z + alt) & 1)) * BY;
  int offz = _bid_z * BZ;

  // halo shared memory coords
  int sx = _tid_x;
  int sy1 = 2*_tid_y;
  int sy2 = 2*_tid_y + 1;
  int sz = _tid_z;

  // global coords
  int x = offx + sx;
  int y1 = offy + sy1;
  int y2 = offy + sy2;
  int z = offz + sz;

  //if(x >= N || y1 >= N || y2 >= N || z >= N)
  //return;

  // global and local and block id in soc
  int tid = z*L*L/4 + (_bid_y * BY/2 + _tid_y)*L + x;
  // shared memory
  site_t ss[SVOLUME];

  // load the spins into shared memory
  ss[sC(sx, sy1, sz, sLx, sLy)] = s[C(x, y1, z, L)];
  ss[sC(sx, sy2, sz, sLx, sLy)] = s[C(x, y2, z, L)];
  // get the h1,h2 values for y1 y2.
  int h1 = H[C(x, y1, z, L)];
  int h2 = H[C(x, y2, z, L)];
  //printf("thread %i   h1=%i   h2=%i\n", tid, h1, h2);

  // ------------------------------------------------
  // halo
  // ------------------------------------------------
  // Y boundary
  if(_tid_y == 0){
    // we also check if we are on the limit of the lattice
    ss[sC(sx, -1, sz, sLx, sLy)] = (offy == 0) ? s[C(x, L-1, z, L)] : s[C(x, offy-1, z, L)];
  }
  if(_tid_y == BY/2-1){
    ss[sC(sx, BY, sz, sLx, sLy)] = (offy == L-BY) ? s[C(x, 0, z, L)] : s[C(x, offy+BY, z, L)];
  }

  // X boundary
  if(_tid_x == 0){
    if(_bid_x == 0){
      ss[sC(-1, sy1, sz, sLx, sLy)] = s[C(L-1, y1, z, L)];
      ss[sC(-1, sy2, sz, sLx, sLy)] = s[C(L-1, y2, z, L)];
    }
    else{
      ss[sC(-1, sy1, sz, sLx, sLy)] = s[C(offx-1, y1, z, L)];
      ss[sC(-1, sy2, sz, sLx, sLy)] = s[C(offx-1, y2, z, L)];
    }
  }
  if(_tid_x == BX-1){
    if(_bid_x == GRID_DIM_X-1){
      ss[sC(BX, sy1, sz, sLx, sLy)] = s[C(0, y1, z, L)];
      ss[sC(BX, sy2, sz, sLx, sLy)] = s[C(0, y2, z, L)];
    }
    else{
      ss[sC(BX, sy1, sz, sLx, sLy)] = s[C(offx+BX, y1, z, L)];
      ss[sC(BX, sy2, sz, sLx, sLy)] = s[C(offx+BX, y2, z, L)];
    }
  }

  // Z boundary
  if(_tid_z == 0){
    if(_bid_z == 0){
      ss[sC(sx, sy1, -1, sLx, sLy)] = s[C(x, y1, L-1, L)];
      ss[sC(sx, sy2, -1, sLx, sLy)] = s[C(x, y2, L-1, L)];
    }
    else{
      ss[sC(sx, sy1, -1, sLx, sLy)] = s[C(x, y1, offz-1, L)];
      ss[sC(sx, sy2, -1, sLx, sLy)] = s[C(x, y2, offz-1, L)];
    }
  }
  if(_tid_z == BZ-1){
    if(_bid_z == GRID_DIM_Z-1){
      ss[sC(sx, sy1, BZ, sLx, sLy)] = s[C(x, y1, 0, L)];
      ss[sC(sx, sy2, BZ, sLx, sLy)] = s[C(x, y2, 0, L)];
    }
    else{
      ss[sC(sx, sy1, BZ, sLx, sLy)] = s[C(x, y1, offz+BZ, L)];
      ss[sC(sx, sy2, BZ, sLx, sLy)] = s[C(x, y2, offz+BZ, L)];
    }
  }

  // get random number state
  uint64_t lstate = state[tid];
  uint64_t linc = inc[tid];
  // the white and black y
  int wy = ((sx + sz) & 1)     + 2*_tid_y;
  int by = ((sx + sz + 1) & 1)  + 2*_tid_y;
  float dh;
  int c;
  //#pragma unroll
  for(int i = 0; i < BLOCK_STEPS; ++i){

    /* -------- white update -------- */
    dh = (float)(ss[sC(sx, wy, sz, sLx, sLy)] * (
         (float)(ss[sC(sx-1,wy,sz, sLx, sLy)] + ss[sC(sx+1, wy, sz, sLx, sLy)] + 
                 ss[sC(sx,wy-1,sz, sLx, sLy)] + ss[sC(sx, wy+1, sz, sLx, sLy)] +
                 ss[sC(sx,wy,sz-1, sLx, sLy)] + ss[sC(sx, wy, sz+1, sLx, sLy)]) + h*h1));
    c = signbit(dh-EPSILON) | signbit(gpu_rand01(&lstate, &linc) - expf(dh * B));
    ss[sC(sx, wy, sz, sLx, sLy)] *= (1 - 2*c);

    /* -------- black update -------- */
    dh = (float)(ss[sC(sx, by, sz, sLx, sLy)] * (
         (float)(ss[sC(sx-1,by,sz, sLx, sLy)] + ss[sC(sx+1, by, sz, sLx, sLy)] + 
                 ss[sC(sx,by-1,sz, sLx, sLy)] + ss[sC(sx, by+1, sz, sLx, sLy)] +
                 ss[sC(sx,by,sz-1, sLx, sLy)] + ss[sC(sx, by, sz+1, sLx, sLy)]) + h*h2));

    c = signbit(dh-EPSILON) | signbit(gpu_rand01(&lstate, &linc) - expf(dh * B));
    ss[sC(sx, by, sz, sLx, sLy)] *= (1 - 2*c);
  }

  /* copy data back to gmem */
  s[C(x, y1, z, L)] = ss[sC(sx, sy1, sz, sLx, sLy)];
  s[C(x, y2, z, L)] = ss[sC(sx, sy2, sz, sLx, sLy)]; 
  /* update random number state */
  state[tid] = lstate;
  inc[tid] = linc;
}

// NOTE: the space of computation is 1/4 of N, so that is why each thread does quadruple work.
void 
kernel_reset_random_gpupcg(int *s, int N, uint64_t *state, uint64_t *inc)
{
  int x = _bid_x * BLOCK_DIM_X + _tid_x;
  float v;
  /* Each thread gets same seed, a different sequence number, no offset */
  if( x >= N/4 ) return;

  /* get the prng state in register memory */
  uint64_t lstate = state[x];
  uint64_t linc = inc[x];

  // first random
  v = (int) (gpu_rand01(&lstate, &linc) + 0.5f);
  s[x] = 1-2*v;
  // second random
  v = (int) (gpu_rand01(&lstate, &linc) + 0.5f);
  s[x + N/4] = 1-2*v;
  // third random
  v = (int) (gpu_rand01(&lstate, &linc) + 0.5f);
  s[x + N/2 ]  = 1-2*v;
  // fourth random
  v = (int) (gpu_rand01(&lstate, &linc) + 0.5f);
  s[x + 3*N/4] = 1-2*v;

  /* save the state back to global memory */
  state[x] = lstate;
  inc[x] = linc;
}

template<typename T>
void kernel_reset(T *a, int N, T val){
  int idx = _bid_x * BLOCK_DIM_X + _tid_x;
  if(idx < N) a[idx] = val;
}
#endif


// --- from kernel_prng.h ---
//////////////////////////////////////////////////////////////////////////////////
//                                                                              //
//  trueke                                                                      //
//  A multi-GPU implementation of the exchange Monte Carlo method.              //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////
//                                                                              //
//  Copyright © 2015 Cristobal A. Navarro, Wei Huang.                           //
//                                                                              //
//  This file is part of trueke.                                                //
//  trueke is free software: you can redistribute it and/or modify              //
//  it under the terms of the GNU General Public License as published by        //
//  the Free Software Foundation, either version 3 of the License, or           //
//  (at your option) any later version.                                         //
//                                                                              //
//  trueke is distributed in the hope that it will be useful,                   //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of              //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               //
//  GNU General Public License for more details.                                //
//                                                                              //
//  You should have received a copy of the GNU General Public License           //
//  along with trueke.  If not, see <http://www.gnu.org/licenses/>.             //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////
#ifndef _KERNEL_PRNG_SETUP_
#define _KERNEL_PRNG_SETUP_

/*
 * PCG Random Number Generation for C.
 *
 * Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For additional information about the PCG random number generation scheme,
 * including its license and other licensing options, visit
 *
 *       http://www.pcg-random.org
 */

/*
 * This code is derived from the full C implementation, which is in turn
 * derived from the canonical C++ PCG implementation. The C++ version
 * has many additional features and is preferable if you can use C++ in
 * your project.
 */

#include <limits.h>
#include <inttypes.h>

#define INV_UINT_MAX 2.3283064e-10f

inline uint32_t gpu_pcg32_random_r(uint64_t *state, uint64_t *inc);

inline void gpu_pcg32_srandom_r(uint64_t *state, uint64_t *inc, uint64_t initstate, uint64_t initseq)
{
  *state = 0U;
  *inc = (initseq << 1u) | 1u;
  gpu_pcg32_random_r(state, inc);
  *state += initstate;
  gpu_pcg32_random_r(state, inc);
}

inline uint32_t gpu_pcg32_random_r(uint64_t *state, uint64_t *inc)
{
  uint64_t oldstate = *state;
  *state = oldstate * 6364136223846793005ULL + *inc;
  uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
  uint32_t rot = oldstate >> 59u;
  return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

inline float gpu_rand01(uint64_t *state, uint64_t *inc)
{
  return (float) gpu_pcg32_random_r(state, inc) * INV_UINT_MAX;
}

// Murmur hash 64-bit
uint64_t mmhash64( const void * key, int len, unsigned int seed )
{
  const uint64_t m = 0xc6a4a7935bd1e995;
  const int r = 47;

  uint64_t h = seed ^ (len * m);

  const uint64_t * data = (const uint64_t *)key;
  const uint64_t * end = data + (len/8);

  while(data != end){
    uint64_t k = *data++;

    k *= m; 
    k ^= k >> r; 
    k *= m; 

    h ^= k;
    h *= m; 
  }
  const unsigned char * data2 = (const unsigned char*)data;
  switch(len & 7)
  {
    case 7: h ^= uint64_t(data2[6]) << 48;
    case 6: h ^= uint64_t(data2[5]) << 40;
    case 5: h ^= uint64_t(data2[4]) << 32;
    case 4: h ^= uint64_t(data2[3]) << 24;
    case 3: h ^= uint64_t(data2[2]) << 16;
    case 2: h ^= uint64_t(data2[1]) << 8;
    case 1: h ^= uint64_t(data2[0]);
      h *= m;
  };

  h ^= h >> r;
  h *= m;
  h ^= h >> r;
  return h;
} 

void kernel_gpupcg_setup(uint64_t *state, uint64_t *inc, int N, 
                         uint64_t seed, uint64_t seq)
{
  int x = _bid_x * BLOCK_DIM_X + _tid_x;
  if( x < N ){
    // exclusive seeds, per replica sequences 
    uint64_t tseed = x + seed;
    uint64_t hseed = mmhash64(&tseed, sizeof(uint64_t), 17);
    uint64_t hseq = mmhash64(&seq, sizeof(uint64_t), 47);
    gpu_pcg32_srandom_r(&state[x], &inc[x], hseed, hseq);
  }
}
#endif



// --- from kernel_reduction.h ---
//  trueke                                                                      //
//  A multi-GPU implementation of the exchange Monte Carlo method.              //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////
//                                                                              //
//  Copyright © 2015 Cristobal A. Navarro, Wei Huang.                           //
//                                                                              //
//  This file is part of trueke.                                                //
//  trueke is free software: you can redistribute it and/or modify              //
//  it under the terms of the GNU General Public License as published by        //
//  the Free Software Foundation, either version 3 of the License, or           //
//  (at your option) any later version.                                         //
//                                                                              //
//  trueke is distributed in the hope that it will be useful,                   //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of              //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               //
//  GNU General Public License for more details.                                //
//                                                                              //
//  You should have received a copy of the GNU General Public License           //
//  along with trueke.  If not, see <http://www.gnu.org/licenses/>.             //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////
#ifndef _REDUCTION_H_
#define _REDUCTION_H_

/* warp reduction with shfl function */
template < typename T >
__inline__ float warp_reduce(T val)
{
  for (int offset = WARPSIZE >> 1; offset > 0; offset >>= 1)
    val += __shfl_down(val, offset);
  return val;
}

/* block reduction with warp reduction */
template < typename T >
__inline__ float block_reduce(T val)
{
  static T shared[WARPSIZE];
  int tid = _tid_z * BY * BX + _tid_y * BX + _tid_x;
  int lane = tid & (WARPSIZE-1);
  int wid = tid/WARPSIZE;
  val = warp_reduce<T>(val);

  if(lane == 0)
    shared[wid] = val;

  val = (tid < (BLOCK_DIM_X * BLOCK_DIM_Y * BLOCK_DIM_Z)/WARPSIZE) ? shared[lane] : 0;
  if(wid == 0){
    val = warp_reduce<T>(val);
  }
  return val;
}

/* energy reduction using block reduction */
template <typename T>
void kernel_redenergy(const int *s, int L, T *out, const int *H, float h)
{
  // offsets
  int x = _bid_x *BLOCK_DIM_X + _tid_x;
  int y = _bid_y *BLOCK_DIM_Y + _tid_y;
  int z = _bid_z *BLOCK_DIM_Z + _tid_z;
  int tid = _tid_z * BY * BX + _tid_y * BX + _tid_x;
  int id = C(x,y,z,L);
  // this optimization only works for L being a power of 2
  //float sum = -(float)(s[id] * ((float)(s[C((x+1) & (L-1), y, z, L)] + 
  // s[C(x, (y+1) & (L-1), z, L)] + s[C(x, y, (z+1) & (L-1), L)]) + h*H[id]));

  // this line works always
  float sum = -(float)(s[id] * ((float)(s[C((x+1) >=  L? 0: x+1, y, z, L)] + 
              s[C(x, (y+1) >= L? 0 : y+1, z, L)] + s[C(x, y, (z+1) >= L? 0 : z+1, L)]) + h*H[id]));
  sum = block_reduce<T>(sum); 

  if(tid == 0) (*out += sum);
}

#endif


// --- from utils.h ---
#ifndef UTILS_H
#define UTILS_H

#include <hip/hip_runtime.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>


#define cudaCheckErrors(msg) \
  do { \
    hipError_t __err = hipGetLastError(); \
    if (__err != hipSuccess) { \
      fprintf(stderr, "Fatal error: %s (%s at %s:%d)\n", \
          msg, hipGetErrorString(__err), __FILE__, __LINE__); \
      fprintf(stderr, "*** FAILED - ABORTING\n"); \
      exit(1); \
    } \
  } while(0)

// same result when warp size is 64
#define WARPSIZE 32

#ifndef BX
#define BX  16
#endif

#ifndef BY
#define BY  8
#endif

#ifndef BZ
#define BZ  4
#endif

#ifndef BLOCKSIZE1D 
#define BLOCKSIZE1D 256
#endif

void printarray(float *a, int n, const char *name);

void printarrayfrag(float *a, int m, const char *name);

void printindexarrayfrag(float *a, findex* ind, int m, const char *name);

void reset_array(float *a, int n, float val);

void fgoleft(findex_t *frag, int ar);

findex_t fgetleft(findex_t frag, int ar);

void newtemp(float *aT, int *ar, int *R, findex_t l);

void rebuild_temps(float *aT, int R, int ar);

void insert_temps(float *aavex, float *aT, int *R, int *ar, int ains);

void rebuild_indices(findex_t* arts, findex_t *atrs, int ar) ;

double rtclock();

#endif
