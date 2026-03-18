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
/*
 *  Extension to the interleaving example in CUDA Programming by Shane Cook
 */
#include <stdio.h>
#include <stdlib.h>
#include <chrono>

#define NUM_ELEMENTS 4096
#define COUNT 4096       // accumulation count

// an interleaved type
typedef struct
{
  unsigned int s0;
  unsigned int s1;
  unsigned int s2;
  unsigned int s3;
  unsigned int s4;
  unsigned int s5;
  unsigned int s6;
  unsigned int s7;
  unsigned int s8;
  unsigned int s9;
  unsigned int sa;
  unsigned int sb;
  unsigned int sc;
  unsigned int sd;
  unsigned int se;
  unsigned int sf;
} INTERLEAVED_T;

// Define an array type based on the interleaved structure
typedef INTERLEAVED_T INTERLEAVED_ARRAY_T[NUM_ELEMENTS];

// Alternative - structure of arrays
typedef unsigned int ARRAY_MEMBER_T[NUM_ELEMENTS];
typedef struct
{
  ARRAY_MEMBER_T s0;
  ARRAY_MEMBER_T s1;
  ARRAY_MEMBER_T s2;
  ARRAY_MEMBER_T s3;
  ARRAY_MEMBER_T s4;
  ARRAY_MEMBER_T s5;
  ARRAY_MEMBER_T s6;
  ARRAY_MEMBER_T s7;
  ARRAY_MEMBER_T s8;
  ARRAY_MEMBER_T s9;
  ARRAY_MEMBER_T sa;
  ARRAY_MEMBER_T sb;
  ARRAY_MEMBER_T sc;
  ARRAY_MEMBER_T sd;
  ARRAY_MEMBER_T se;
  ARRAY_MEMBER_T sf;
} NON_INTERLEAVED_T;

// data initialization and verification







// --- from util.h ---
#include <assert.h>

void initialize( INTERLEAVED_ARRAY_T &interleaved_src,
                 INTERLEAVED_ARRAY_T &interleaved_dst,
		 NON_INTERLEAVED_T &non_interleaved_src,
		 NON_INTERLEAVED_T &non_interleaved_dst,
	         const int n )
{
  for (int i = 0; i < n; i++) {
    interleaved_src[i].s0 = non_interleaved_src.s0[i] = rand() % 16;
    interleaved_src[i].s1 = non_interleaved_src.s1[i] = rand() % 16;
    interleaved_src[i].s2 = non_interleaved_src.s2[i] = rand() % 16;
    interleaved_src[i].s3 = non_interleaved_src.s3[i] = rand() % 16;
    interleaved_src[i].s4 = non_interleaved_src.s4[i] = rand() % 16;
    interleaved_src[i].s5 = non_interleaved_src.s5[i] = rand() % 16;
    interleaved_src[i].s6 = non_interleaved_src.s6[i] = rand() % 16;
    interleaved_src[i].s7 = non_interleaved_src.s7[i] = rand() % 16;
    interleaved_src[i].s8 = non_interleaved_src.s8[i] = rand() % 16;
    interleaved_src[i].s9 = non_interleaved_src.s9[i] = rand() % 16;
    interleaved_src[i].sa = non_interleaved_src.sa[i] = rand() % 16;
    interleaved_src[i].sb = non_interleaved_src.sb[i] = rand() % 16;
    interleaved_src[i].sc = non_interleaved_src.sc[i] = rand() % 16;
    interleaved_src[i].sd = non_interleaved_src.sd[i] = rand() % 16;
    interleaved_src[i].se = non_interleaved_src.se[i] = rand() % 16;
    interleaved_src[i].sf = non_interleaved_src.sf[i] = rand() % 16;
    interleaved_dst[i].s0 = non_interleaved_dst.s0[i] = 0;
    interleaved_dst[i].s1 = non_interleaved_dst.s1[i] = 0;
    interleaved_dst[i].s2 = non_interleaved_dst.s2[i] = 0;
    interleaved_dst[i].s3 = non_interleaved_dst.s3[i] = 0;
    interleaved_dst[i].s4 = non_interleaved_dst.s4[i] = 0;
    interleaved_dst[i].s5 = non_interleaved_dst.s5[i] = 0;
    interleaved_dst[i].s6 = non_interleaved_dst.s6[i] = 0;
    interleaved_dst[i].s7 = non_interleaved_dst.s7[i] = 0;
    interleaved_dst[i].s8 = non_interleaved_dst.s8[i] = 0;
    interleaved_dst[i].s9 = non_interleaved_dst.s9[i] = 0;
    interleaved_dst[i].sa = non_interleaved_dst.sa[i] = 0;
    interleaved_dst[i].sb = non_interleaved_dst.sb[i] = 0;
    interleaved_dst[i].sc = non_interleaved_dst.sc[i] = 0;
    interleaved_dst[i].sd = non_interleaved_dst.sd[i] = 0;
    interleaved_dst[i].se = non_interleaved_dst.se[i] = 0;
    interleaved_dst[i].sf = non_interleaved_dst.sf[i] = 0;
  }
}

void verify( INTERLEAVED_ARRAY_T &interleaved_dst, 
             NON_INTERLEAVED_T &non_interleaved_dst, const int n )
{
  for (int i = 0; i < n; i++) {
    assert(interleaved_dst[i].s0 == non_interleaved_dst.s0[i]);
    assert(interleaved_dst[i].s1 == non_interleaved_dst.s1[i]);
    assert(interleaved_dst[i].s2 == non_interleaved_dst.s2[i]);
    assert(interleaved_dst[i].s3 == non_interleaved_dst.s3[i]);
    assert(interleaved_dst[i].s4 == non_interleaved_dst.s4[i]);
    assert(interleaved_dst[i].s5 == non_interleaved_dst.s5[i]);
    assert(interleaved_dst[i].s6 == non_interleaved_dst.s6[i]);
    assert(interleaved_dst[i].s7 == non_interleaved_dst.s7[i]);
    assert(interleaved_dst[i].s8 == non_interleaved_dst.s8[i]);
    assert(interleaved_dst[i].s9 == non_interleaved_dst.s9[i]);
    assert(interleaved_dst[i].sa == non_interleaved_dst.sa[i]);
    assert(interleaved_dst[i].sb == non_interleaved_dst.sb[i]);
    assert(interleaved_dst[i].sc == non_interleaved_dst.sc[i]);
    assert(interleaved_dst[i].sd == non_interleaved_dst.sd[i]);
    assert(interleaved_dst[i].se == non_interleaved_dst.se[i]);
    assert(interleaved_dst[i].sf == non_interleaved_dst.sf[i]);
  }
}
