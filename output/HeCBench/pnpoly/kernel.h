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
#include <string.h>
#include <math.h>
#include <random>
#include <chrono>

#define VERTICES 600
#define BLOCK_SIZE_X 256




// --- from kernel.h ---
/*
 * This file contains the implementation of a kernel for the
 * point-in-polygon problem using the crossing number algorithm
 *
 * The kernel pnpoly_base is used for correctness checking.
 *
 * The algorithm used here is adapted from: 
 *     'Inclusion of a Point in a Polygon', Dan Sunday, 2001
 *     (http://geomalgorithms.com/a03-_inclusion.html)
 *
 * Author: Ben van Werkhoven <b.vanwerkhoven@esciencecenter.nl>
 */

/*
 * The is_between method returns a boolean that is True when the a is between c and b.
 */
inline int is_between(float a, float b, float c) {
  return (b > a) != (c > a);
}

/*
 * The Point-in-Polygon kernel
 */
template <int tile_size>
void pnpoly_opt(
    int* bitmap,
    const float2* point,
    const float2* vertex,
    int n) 
{
  int i = _bid_x * BLOCK_SIZE_X * tile_size + _tid_x;
  if (i < n) {
    int c[tile_size];
    float2 lpoint[tile_size];
    #pragma unroll
    for (int ti=0; ti<tile_size; ti++) {
      c[ti] = 0;
      if (i+BLOCK_SIZE_X*ti < n) {
        lpoint[ti] = point[i+BLOCK_SIZE_X*ti];
      }
    }

    int k = VERTICES-1;

    for (int j=0; j<VERTICES; k = j++) {    // edge from vj to vk
      float2 vj = vertex[j]; 
      float2 vk = vertex[k]; 

      float slope = (vk.x-vj.x) / (vk.y-vj.y);

      #pragma unroll
      for (int ti=0; ti<tile_size; ti++) {

        float2 p = lpoint[ti];

        if (is_between(p.y, vj.y, vk.y) &&         //if p is between vj and vk vertically
            (p.x < slope * (p.y-vj.y) + vj.x)
           ) {  //if p.x crosses the line vj-vk when moved in positive x-direction
          c[ti] = !c[ti];
        }
      }
    }

    #pragma unroll
    for (int ti=0; ti<tile_size; ti++) {
      //could do an if statement here if 1s are expected to be rare
      if (i+BLOCK_SIZE_X*ti < n)
        bitmap[i+BLOCK_SIZE_X*ti] = c[ti];
    }
  }
}

/*
 * The naive implementation is used for verifying correctness of the optimized implementation
 */
void pnpoly_base(
    int* bitmap,
    const float2* point,
    const float2* vertex,
    int n) 
{
  int i = _bid_x * BLOCK_DIM_X + _tid_x;
  if (i < n) {
    int c = 0;
    float2 p = point[i];

    int k = VERTICES-1;

    for (int j=0; j<VERTICES; k = j++) {    // edge from v to vp
      float2 vj = vertex[j]; 
      float2 vk = vertex[k]; 

      float slope = (vk.x-vj.x) / (vk.y-vj.y);

      if (((vj.y>p.y) != (vk.y>p.y)) &&            //if p is between vj and vk vertically
          (p.x < slope * (p.y-vj.y) + vj.x)) {   //if p.x crosses the line vj-vk when moved in positive x-direction
        c = !c;
      }
    }

    bitmap[i] = c; // 0 if even (out), and 1 if odd (in)
  }
}

