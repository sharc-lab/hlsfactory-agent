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

// --- from bench_merkle_tree.cu ---
#include <random>
#include <chrono>
#include "bench_merkle_tree.hpp"
#include "merkle_tree.hpp"

uint64_t
benchmark_merklize_approach_1(const size_t leaf_count,
                              const size_t wg_size)
{
  const size_t leaves_size = sizeof(ulong) * leaf_count * DIGEST_SIZE;
  const size_t mds_size = sizeof(ulong4) * STATE_WIDTH * 3;
  const size_t ark_size = sizeof(ulong4) * NUM_ROUNDS * 3;

  ulong* leaves_h;
  cudaMallocHost((void**)&leaves_h, leaves_size);

  ulong* leaves_d;

  ulong* intermediates_d;

  ulong4* mds_h;
  cudaMallocHost((void**)&mds_h, mds_size);

  ulong4* mds_d;

  ulong4* ark1_h;
  cudaMallocHost((void**)&ark1_h, ark_size);

  ulong4* ark1_d;

  ulong4* ark2_h;
  cudaMallocHost((void**)&ark2_h, ark_size);

  ulong4* ark2_d;

  {
    std::mt19937 gen(19937);
    std::uniform_int_distribution<uint64_t> dis(1ul, MOD);

  }

  prepare_mds(mds_h);
  prepare_ark1(ark1_h);
  prepare_ark2(ark2_h);

  // this itself does host synchronization
  auto start_time = std::chrono::high_resolution_clock::now();
  merklize_approach_1(
    leaves_d, intermediates_d, leaf_count, wg_size, mds_d, ark1_d, ark2_d);
  auto end_time = std::chrono::high_resolution_clock::now();
  uint64_t ts = std::chrono::duration_cast<std::chrono::nanoseconds>(
                end_time - start_time).count();

#ifdef DEBUG
  ulong *intermediates_h = (ulong*) malloc (leaves_size);

  free(intermediates_h);
#endif

  cudaFreeHost(leaves_h);
  cudaFreeHost(mds_h);
  cudaFreeHost(ark1_h);
  cudaFreeHost(ark2_h);

  return ts;
}


// --- from ff_p.cu ---
#include "ff_p.hpp"
#include <climits>

uint64_t
ff_p_add(uint64_t a, uint64_t b)
{

  uint64_t res_0 = a + b;
  bool over_0 = a > UINT64_MAX - b;

  uint32_t zero = 0;
  uint64_t tmp_0 = (uint64_t)(zero - (uint32_t)(over_0 ? 1 : 0));

  uint64_t res_1 = res_0 + tmp_0;
  bool over_1 = res_0 > UINT64_MAX - tmp_0;

  uint64_t tmp_1 = (uint64_t)(zero - (uint32_t)(over_1 ? 1 : 0));
  uint64_t res = res_1 + tmp_1;

  return res;
}

uint64_t
ff_p_sub(uint64_t a, uint64_t b)
{

  uint64_t res_0 = a - b;
  bool under_0 = a < b;

  uint32_t zero = 0;
  uint64_t tmp_0 = (uint64_t)(zero - (uint32_t)(under_0 ? 1 : 0));

  uint64_t res_1 = res_0 - tmp_0;
  bool under_1 = res_0 < tmp_0;

  uint64_t tmp_1 = (uint64_t)(zero - (uint32_t)(under_1 ? 1 : 0));
  uint64_t res = res_1 + tmp_1;

  return res;
}

uint64_t
ff_p_mult(uint64_t a, uint64_t b)
{

  uint64_t ab = a * b;
  uint64_t cd = __umul64hi(a, b);
  uint64_t c = cd & 0x00000000ffffffff;
  uint64_t d = cd >> 32;

  uint64_t res_0 = ab - d;
  bool under_0 = ab < d;

  uint32_t zero = 0;
  uint64_t tmp_0 = (uint64_t)(zero - (uint32_t)(under_0 ? 1 : 0));
  res_0 -= tmp_0;

  uint64_t tmp_1 = (c << 32) - c;

  uint64_t res_1 = res_0 + tmp_1;
  bool over_0 = res_0 > UINT64_MAX - tmp_1;

  uint64_t tmp_2 = (uint64_t)(zero - (uint32_t)(over_0 ? 1 : 0));
  uint64_t res = res_1 + tmp_2;

  return res;
}

uint64_t
ff_p_pow(uint64_t a, const uint64_t b)
{



  uint64_t r = b & 0b1 ? a : 1;
  return r;
}

uint64_t
ff_p_inv(uint64_t a)
{


  const uint64_t exp = MOD - 2;


  uint64_t b_inv = ff_p_inv(b);
  return ff_p_mult(a, b_inv);
}


// --- from main.cu ---
#include <iostream>
#include <iomanip>
#include "bench_merkle_tree.hpp"

#ifdef DEBUG
const size_t BENCH_ROUND = 1;
#else
const size_t BENCH_ROUND = 4;
#endif


// --- from merkle_tree.cu ---
#include <stdio.h>
#include "merkle_tree.hpp"


  


// --- from rescue_prime.cu ---
#include "rescue_prime.hpp"


inline ulong4 operator*(const ulong4 &a, const ulong4 &b)
{
  return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

inline ulong4 operator+(const ulong4 &a, const ulong4 &b)
{
  return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

inline ulong4 operator-(const ulong4 &a, const ulong4 &b)
{
  return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

inline ulong4 operator-(const ulong &a, const ulong4 &b)
{
  return {a - b.x, a - b.y, a - b.z, a - b.w};
}

inline ulong4 operator&(const ulong4 &a, const ulong b)
{
  return {a.x & b, a.y & b, a.z & b, a.w & b};
}

inline ulong4 operator>>(const ulong4 &a, const int b)
{
  return {a.x >> b, a.y >> b, a.z >> b, a.w >> b};
}

inline ulong4 operator<<(const ulong4 &a, const int b)
{
  return {a.x << b, a.y << b, a.z << b, a.w << b};
}

inline ulong4 operator<(const ulong4 &a, const ulong4 &b)
{
  return {(a.x < b.x) ? ULONG_MAX : 0, 
          (a.y < b.y) ? ULONG_MAX : 0,
          (a.z < b.z) ? ULONG_MAX : 0,
          (a.w < b.w) ? ULONG_MAX : 0};
}

inline ulong4 operator>(const ulong4 &a, const ulong4 &b)
{
  return {(a.x > b.x) ? ULONG_MAX : 0, 
          (a.y > b.y) ? ULONG_MAX : 0,
          (a.z > b.z) ? ULONG_MAX : 0,
          (a.w > b.w) ? ULONG_MAX : 0};
}

inline ulong4 operator>=(const ulong4 &a, const ulong4 &b)
{
  return {(a.x >= b.x) ? ULONG_MAX : 0, 
          (a.y >= b.y) ? ULONG_MAX : 0,
          (a.z >= b.z) ? ULONG_MAX : 0,
          (a.w >= b.w) ? ULONG_MAX : 0};
}

ulong4
ff_p_vec_mul_(ulong4 a, ulong4 b)
{
  ulong4 ab = a * b;
  ulong4 cd = mul_hi(a, b);
  ulong4 c = cd & MAX_UINT;
  ulong4 d = cd >> 32;

  ulong4 tmp_0 = ab - d;

  ulong4 und_0 = ab < d; // check if underflowed
  ulong4 tmp_2 = und_0 & MAX_UINT;

  ulong4 tmp_3 = tmp_0 - tmp_2;

  ulong4 tmp_4 = (c << 32) - c;

  ulong4 tmp_5 = tmp_3 + tmp_4;
  ulong4 ovr_0 = tmp_3 > (std::numeric_limits<uint64_t>::max() - tmp_4);
  ulong4 tmp_7 = ovr_0 & MAX_UINT;

  return tmp_5 + tmp_7;
}


ulong4
ff_p_vec_add_(const ulong4 &a, const ulong4 &b)
{
  // Following four lines are equivalent of writing
  // b % FIELD_MOD, which converts all lanes of `b` vector
  // into canonical representation
  const ulong4 mod_vec = make_ulong4(MOD, MOD, MOD, MOD);
  ulong4 over_0 = b >= mod_vec;
  ulong4 tmp_0 = (over_0 >> 63) * mod_vec;
  ulong4 b_ok = b - tmp_0;

  ulong4 tmp_1 = a + b_ok;
  ulong4 over_1 = a > (std::numeric_limits<uint64_t>::max() - b_ok);
  ulong4 tmp_2 = over_1 & MAX_UINT;

  ulong4 tmp_3 = tmp_1 + tmp_2;
  ulong4 over_2 = tmp_1 > (std::numeric_limits<uint64_t>::max() - tmp_2);
  ulong4 tmp_4 = over_2 & MAX_UINT;

  return tmp_3 + tmp_4;
}




ulong
accumulate_vec4(ulong4 a)
{
  uint64_t v0 = ff_p_add(a.x, a.y);
  uint64_t v1 = ff_p_add(a.z, a.w);

  return static_cast<ulong>(ff_p_add(v0, v1));
}

ulong
accumulate_state(const ulong4* state)
{
  ulong v0 = accumulate_vec4(*(state + 0));
  ulong v1 = accumulate_vec4(*(state + 1));
  ulong v2 = accumulate_vec4(*(state + 2));

  return static_cast<ulong>(ff_p_add(v2, ff_p_add(v0, v1)));
}










