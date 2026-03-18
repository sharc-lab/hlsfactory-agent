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
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <chrono>




// --- from benchmark.h ---
#if !defined(_MSC_VER) || _MSC_VER >= 1800 // Visual Studio 2013 is the first version with inttypes.h
#include <inttypes.h>
#else
#define PRIu64 "llu"
#define PRIu32 "u"
#endif

#ifdef _MSC_VER
#define inline __inline
#endif

#if (defined(__CUDACC__) || defined (__HIPCC__))
#define ESS #else
#define ESS
#endif

#define BENCHMARK_ITERATIONS 100000

// With hashing only 3 bases are enough to test all numbers up to 2^64
// and 1 base to test all up to 2^32
// see http://probableprime.org/download/example-primality.c
#define BASES_CNT32 3
#define BASES_CNT_MAX BASES_CNT32



// found by Gerhard Jaeschke
const uint32_t bases32[] = {2, 7, 61};
// see http://miller-rabin.appspot.com

#define SIZES_CNT32 4
#define SIZES_CNT_MAX SIZES_CNT32

static const char bits32[SIZES_CNT32] = {8,16,24,32};
static const uint32_t mask32[SIZES_CNT32] = {0xFFU,0xFFFFU,0xFFFFFFU,0xFFFFFFFFU};
static uint32_t n32[SIZES_CNT32][BENCHMARK_ITERATIONS];

#define WHEEL_PRODUCT 105
// wheel contains only odd numbers
static const unsigned char distancewheel[WHEEL_PRODUCT] = 
{0,8,6,4,2,0,0,2,0,0,2,0,4,2,0,0,4,2,0,2,0,0,2,0,4,2,0,4,2,0,0,4,2,0,2,
  0,0,4,2,0,2,0,4,2,0,6,4,2,0,2,0,0,2,0,0,2,0,6,4,2,0,4,2,0,2,0,4,2,0,0,
  2,0,4,2,0,0,4,2,0,4,2,0,2,0,0,2,0,4,2,0,0,4,2,0,2,0,0,2,0,0,8,6,4,2,0};
static const unsigned char wheeladvance[WHEEL_PRODUCT] = 
{10,0,0,0,0,2,4,0,2,4,0,6,0,0,2,6,0,0,4,0,2,4,0,6,0,0,6,0,0,2,6,0,0,4,0,
  2,6,0,0,4,0,6,0,0,8,0,0,0,4,0,2,4,0,2,4,0,8,0,0,0,6,0,0,4,0,6,0,0,2,4,
  0,6,0,0,2,6,0,0,6,0,0,4,0,2,4,0,6,0,0,2,6,0,0,4,0,2,4,0,2,10,0,0,0,0,2};

static void set_nprimes()
{
  myseed();
  for (int i = 0; i < SIZES_CNT32; i++)
    // simple PRIMEINC method - uniformity isn't important
    for (int j = 0; j < BENCHMARK_ITERATIONS; j++) {
      uint32_t n = (myrand32() & mask32[i]) | 1;
      n += distancewheel[(n >> 1) % WHEEL_PRODUCT];
      if (n < 5) n = 5;
      while (!efficient_mr32(bases32, 3, n))
        n += wheeladvance[(n >> 1) % WHEEL_PRODUCT];
      n32[i][j] = n;
    }
}

static void set_nintegers()
{
  myseed();
  for (int i = 0; i < SIZES_CNT32; i++)
    for (int j = 0; j < BENCHMARK_ITERATIONS; j++) {
      uint32_t n = (myrand32() & mask32[i]) | 1;
      if (n < 5) n = 5;
      n32[i][j] = n;
    }
}

void print_results(const char *bits_array, const int bits_limit, const int cnt_limit, uint64_t time_vals[][3][2])
{
  int i, j;

  printf("         ");
  for (i = 0; i < bits_limit; i++) {
    printf("|    %2d-bit integer   ", bits_array[i]);
  }
  printf("\n  bases  ");
  for (i = 0; i < bits_limit; i++) {
    printf("|  effcnt  |  simple  ");
  }
  printf("\n");

  for (i = 0; i < cnt_limit; i++) {
    const int cnt = i + 1;

    printf(" %d base%s", cnt, (cnt != 1 ? "s" : " "));

    for (j = 0; j < bits_limit; j++) {
      printf(" | %5" PRIu64 " ns", time_vals[j][i][0] / BENCHMARK_ITERATIONS);
      printf(" | %5" PRIu64 " ns", time_vals[j][i][1] / BENCHMARK_ITERATIONS);
    }
    printf("\n");
  }
  printf("\n");
}



// --- from kernels.h ---
void mr32_sf(
  const uint32_t * bases,
  const uint32_t * n32,
  int * val,
  int iter)
{
  int j = _bid_x * BLOCK_DIM_X + _tid_x;
  if (j < iter) {
    int n = n32[j];
    for (int cnt = 1; cnt <= BASES_CNT32; cnt++) {
      (*val += straightforward_mr32(bases, cnt, n));
    }
  }
}

void mr32_eff(
  const uint32_t * bases,
  const uint32_t * n32,
  int * val,
  int iter)
{
  int j = _bid_x * BLOCK_DIM_X + _tid_x;
  if (j < iter) {
    int n = n32[j];
    for (int cnt = 1; cnt <= BASES_CNT32; cnt++) {
      (*val += efficient_mr32(bases, cnt, n));
    }
  }
}


// --- from myrand.h ---
#ifndef _MYRAND_H_INCLUDED
#define _MYRAND_H_INCLUDED

// xorshift random number generator by George Marsaglia
// it has period of 2^128 - 1
// see http://en.wikipedia.org/wiki/Xorshift

uint32_t __rand_x = 123456789;
uint32_t __rand_y = 362436069;
uint32_t __rand_z = 521288629;
uint32_t __rand_w = 88675123;

void myseed()
{
	__rand_x = 123456789;
	__rand_y = 362436069;
	__rand_z = 521288629;
	__rand_w = 88675123;
}

uint32_t myrand32()
{
	uint32_t t;

	t = __rand_x ^ (__rand_x << 11);
	__rand_x = __rand_y; __rand_y = __rand_z; __rand_z = __rand_w;
	return __rand_w = __rand_w ^ (__rand_w >> 19) ^ (t ^ (t >> 8));
}

uint64_t myrand64()
{
	return (((uint64_t)myrand32()) << 32) + myrand32();
}

#endif // _MYRAND_H_INCLUDED



// --- from mytime.h ---
#ifndef _MYTIME_H_INCLUDED
#define _MYTIME_H_INCLUDED

#ifdef _MSC_VER
#include <windows.h>

typedef LARGE_INTEGER time_point;

static inline time_point get_time()
{
	LARGE_INTEGER res;
	QueryPerformanceCounter(&res);
	return res;
}

uint64_t diff_time(const LARGE_INTEGER t2, const LARGE_INTEGER t1)
{
	LARGE_INTEGER cycles;
	double cycles_per_ns;
	QueryPerformanceFrequency(&cycles);

	cycles_per_ns = cycles.QuadPart / 1000000000.0;

	return (double)(t2.QuadPart-t1.QuadPart) / cycles_per_ns;
}

uint64_t elapsed_time(const time_point start)
{
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	return diff_time(end, start);
}
#else
typedef struct timespec time_point;

static inline time_point get_time()
{
	struct timespec res;

	clock_gettime(CLOCK_MONOTONIC, &res);

	return res;
}

uint64_t elapsed_time(const time_point start)
{
	struct timespec end;
	clock_gettime(CLOCK_MONOTONIC, &end);

	long seconds  = end.tv_sec  - start.tv_sec;
	long nseconds = end.tv_nsec - start.tv_nsec;

	return seconds * 1000000000ULL + nseconds;
}
#endif

#endif // _MYTIME_H_INCLUDED


// --- from sprp32.h ---
#ifndef _SPRP32_H_INCLUDED
#define _SPRP32_H_INCLUDED

#include <stdint.h>

#ifdef _OPENMP
#pragma omp declare target
#endif

ESS
static inline uint32_t mont_prod32(const uint32_t a, const uint32_t b, const uint32_t n, const uint32_t npi)
{
  const uint64_t t = (uint64_t)a*b;
  const uint32_t m = (uint32_t)((uint32_t)t*npi);
  const uint32_t u = (t + (uint64_t)m*n) >> 32; // (t + m*n may overflow)

#ifndef SPRP32_ONE_FREE_BIT
  // overflow fix
  if (u < (t >> 32))
    return (uint32_t)(u-n);
#endif

  return u >= n ? (uint32_t)(u-n) : u;
}

ESS
static inline uint32_t mont_square32(const uint32_t a, const uint32_t n, const uint32_t npi)
{
  return mont_prod32(a, a, n, npi);
}

// WARNING: a must be odd
// returns -a^-1 mod 2^32
ESS
static inline uint32_t modular_inverse32(const uint32_t a)
{
  const unsigned char mask[128] = {255,85,51,73,199,93,59,17,15,229,195,89,215,237,203,33,31,117,83,105,231,125,91,49,47,5,227,121,247,13,235,65,63,149,115,137,7,157,123,81,79,37,3,153,23,45,11,97,95,181,147,169,39,189,155,113,111,69,35,185,55,77,43,129,127,213,179,201,71,221,187,145,143,101,67,217,87,109,75,161,159,245,211,233,103,253,219,177,175,133,99,249,119,141,107,193,191,21,243,9,135,29,251,209,207,165,131,25,151,173,139,225,223,53,19,41,167,61,27,241,239,197,163,57,183,205,171,1};

  // use Hensel lifting, suggested by Robert Gerbicz
  uint32_t ret = mask[(a >> 1) & 127];
  ret *= 2 + a * ret;
  ret *= 2 + a * ret;
  return ret;
}

// returns 2^32 mod n
ESS
static inline uint32_t compute_modn32(const uint32_t n)
{
  if (n <= (1U << 31)) {
    uint32_t res = ((1U << 31) % n) << 1;
    return res < n ? res : res-n;
  } else
    return -n;
}

#define PRIME 1
#define COMPOSITE 0

ESS
static inline int efficient_mr32(const uint32_t bases[], const int cnt, const uint32_t n)
{
  const unsigned npi = modular_inverse32(n);
  const unsigned r = compute_modn32(n);
  uint32_t u=n-1;
  const uint32_t nr = n-r;

  int t=0, j;

  while (!(u&1)) { // while even
    t++;
    u >>= 1;
  }

  for (j=0; j<cnt; j++) {
    const uint32_t a = bases[j];
    uint32_t d=r, u_copy = u;

    uint32_t A=((uint64_t)a<<32) % n;
    int i;

    if (!A) continue; // PRIME in subtest

    // compute a^u mod n

    do {
      if (u_copy & 1) d=mont_prod32(d, A, n, npi);
      A=mont_square32(A, n, npi);
    } while (u_copy>>=1);

    if (d == r || d == nr) continue; // PRIME in subtest

    for (i=1; i<t; i++) {
      d=mont_square32(d, n, npi);
      if (d == r) return COMPOSITE;
      if (d == nr) break; // PRIME in subtest
    }

    if (i == t)
      return COMPOSITE;
  }

  return PRIME;
}
#undef PRIME
#undef COMPOSITE
#ifdef _OPENMP
#pragma omp end declare target
#endif

#endif // _SPRP32_H_INCLUDED


// --- from sprp32_sf.h ---
#ifndef _SPRP32_SF_H_INCLUDED
#define _SPRP32_SF_H_INCLUDED

#include <stdint.h>

// 32-bit straightforward implementation begins
#ifdef _OPENMP
#pragma omp declare target
#endif

// we could use uint32_t d and A, but this is faster (at least on x86-64)
ESS
static inline uint32_t modular_exponentiation32(uint32_t a, uint32_t b, uint32_t n)
{
  uint64_t d=1, A=a;
  do {
    if (b&1)
      d=(d*A)%n;
    A=(A*A)%n;
  } while (b>>=1);

  return (uint32_t)d;
}

ESS
static inline uint32_t square_modulo32(uint32_t a, uint32_t n)
{
  return (uint32_t)(((uint64_t)a*a) % n);
}

ESS
static inline int straightforward_mr32(const uint32_t bases[], int bases_cnt, uint32_t n)
{
  uint32_t u=n-1;
  int t=0, j;

  while (u % 2 == 0) { // while even
    t++;
    u >>= 1;
  }

  for (j=0; j<bases_cnt; j++) {
    uint32_t a = bases[j], x;
    int i;

    if (a >= n) a %= n;

    if (a == 0) continue;

    x = modular_exponentiation32(a, u, n);

    if (x == 1 || x == n-1) continue;

    for (i=1; i<t; i++) {
      x=square_modulo32(x, n);
      if (x == 1)   return 0;
      if (x == n-1) break;
    }

    // if we didn't break, the number is composite
    if (i == t) return 0;
  }

  return 1;
}
#ifdef _OPENMP
#pragma omp end declare target
#endif

#endif // _SPRP32_SF_H_INCLUDED
