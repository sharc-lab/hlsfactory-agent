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
#include <math.h>
#include <random>
#include <chrono>

void bond_wlcpowallvisc(
             r64*  force_x,
             r64*  force_y,
             r64*  force_z,
    const float4*  coord_merged,
    const float4*  veloc,
    const int*   nbond,
    const int2*  bonds,
    const r64*  bond_r0,
    const r32*  temp_global,
    const r32*  r0_global,
    const r32*  mu_targ_global,
    const r32*  qp_global,
    const r32*  gamc_global,
    const r32*  gamt_global,
    const r32*  sigc_global,
    const r32*  sigt_global,
    const float3 period,
    const int padding,
    const int n_type,
    const int n_local )
{
  r32 shared_data[4096];
  r32* temp    = &shared_data[0];
  r32* r0      = &shared_data[1*(n_type+1)];
  r32* mu_targ = &shared_data[2*(n_type+1)];
  r32* qp      = &shared_data[3*(n_type+1)];
  r32* gamc    = &shared_data[4*(n_type+1)];
  r32* gamt    = &shared_data[5*(n_type+1)];
  r32* sigc    = &shared_data[6*(n_type+1)];
  r32* sigt    = &shared_data[7*(n_type+1)];


}

template <typename T>

template <typename T>

template <typename T>

template <typename T>

template <typename T>



// --- from utils.h ---
typedef float r32;
typedef double r64;
typedef int i32;

__inline__ float minimum_image( float dr, float p )
{
  float p_half = p * 0.5f;
  return dr + ( dr > -p_half ? ( dr < p_half ? 0.f : -p ) : p );
}

#define _LN_2           6.9314718055994528623E-1
#define _2_TO_MINUS_31  4.6566128730773925781E-10
#define _2_TO_MINUS_32  2.3283064365386962891E-10
#define _TEA_K0      0xA341316C
#define _TEA_K1      0xC8013EA4
#define _TEA_K2      0xAD90777D
#define _TEA_K3      0x7E95761E
#define _TEA_DT      0x9E3779B9

template<typename T>
__inline__ T bound( T x, T lower, T upper )
{
  return max( lower, min( x, upper ) );
}

template<int N> __inline__ void __TEA_core( uint &v0, uint &v1, uint sum = 0 )
{
  sum += _TEA_DT;
  v0 += ( ( v1 << 4 ) + _TEA_K0 ) ^ ( v1 + sum ) ^ ( ( v1 >> 5 ) + _TEA_K1 );
  v1 += ( ( v0 << 4 ) + _TEA_K2 ) ^ ( v0 + sum ) ^ ( ( v0 >> 5 ) + _TEA_K3 );
  __TEA_core < N - 1 > ( v0, v1, sum );
}

template<> __inline__ void __TEA_core<0>( uint &v0, uint &v1, uint sum ) {}

template<int N> __inline__ float gaussian_TEA_fast( bool pred, int u, int v )
{
  uint v0 =  pred ? u : v;
  uint v1 = !pred ? u : v;
  __TEA_core<N>( v0, v1 );
  float f = sinpif( int( v0 ) * float(_2_TO_MINUS_31) );
  float r = sqrtf( -2.0f * float(_LN_2) * log2f( v1 * float(_2_TO_MINUS_32) ) );
  return bound( r * f, -4.0f, 4.0f );
}
