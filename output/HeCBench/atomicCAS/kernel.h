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
 * Copyright 2010 by NVIDIA Corporation.  All rights reserved.  All
 * information contained herein is proprietary and confidential to NVIDIA
 * Corporation.  Any use, reproduction, or disclosure without the written
 * permission of NVIDIA Corporation is prohibited.
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <chrono>

#define NUM_BLOCKS 1024
#define BLOCK_SIZE 256

template <typename T>

template <typename T>

template <typename T>



// --- from kernels.h ---
// int64 atomic_min
inline
long long atomic_min(long long *address, long long val)
{
  long long ret = *address;
  while(val < ret)
  {
    long long old = ret;
    if((ret = atomicCAS((unsigned long long *)address, (unsigned long long)old, (unsigned long long)val)) == old)
      break;
  }
  return ret;
}

// uint64 atomic_min
inline
unsigned long long atomic_min(unsigned long long *address, unsigned long long val)
{
  unsigned long long ret = *address;
  while(val < ret)
  {
    unsigned long long old = ret;
    if((ret = atomicCAS(address, old, val)) == old)
      break;
  }
  return ret;
}

// int64 atomic add
inline
long long atomic_add(long long *address, long long val)
{
  long long old, newdbl, ret = *address;
  do {
    old = ret;
    newdbl = old+val;
  } while((ret = (long long)atomicCAS((unsigned long long*)address, (unsigned long long)old, (unsigned long long)newdbl)) != old);
  return ret;
}

// int64 atomic_max
inline
long long atomic_max(long long *address, long long val)
{
  long long ret = *address;
  while(val > ret)
  {
    long long old = ret;
    if((ret = (long long)atomicCAS((unsigned long long *)address, (unsigned long long)old, (unsigned long long)val)) == old)
      break;
  }
  return ret;
}

// uint64 atomic_max
inline
unsigned long long atomic_max(unsigned long long *address, unsigned long long val)
{
  unsigned long long ret = *address;
  while(val > ret)
  {
    unsigned long long old = ret;
    if((ret = atomicCAS(address, old, val)) == old)
      break;
  }
  return ret;
}

// uint64 atomic add
inline
unsigned long long atomic_add(unsigned long long *address, unsigned long long val)
{
  unsigned long long old, newdbl, ret = *address;
  do {
    old = ret;
    newdbl = old+val;
  } while((ret = atomicCAS(address, old, newdbl)) != old);
  return ret;
}

// For all double atomics:
//      Must do the compare with integers, not floating point,
//      since NaN is never equal to any other NaN

// double atomic_min
inline
double atomic_min(double *address, double val)
{
  unsigned long long ret = __double_as_longlong(*address);
  while(val < __longlong_as_double(ret))
  {
    unsigned long long old = ret;
    if((ret = atomicCAS((unsigned long long *)address, old, __double_as_longlong(val))) == old)
      break;
  }
  return __longlong_as_double(ret);
}

// double atomic_max
inline
double atomic_max(double *address, double val)
{
  unsigned long long ret = __double_as_longlong(*address);
  while(val > __longlong_as_double(ret))
  {
    unsigned long long old = ret;
    if((ret = atomicCAS((unsigned long long *)address, old, __double_as_longlong(val))) == old)
      break;
  }
  return __longlong_as_double(ret);
}

// Double-precision floating point atomic add
inline
double atomic_add(double *address, double val)
{
  // Doing it all as longlongs cuts one __longlong_as_double from the inner loop
  unsigned long long *ptr = (unsigned long long *)address;
  unsigned long long old, newdbl, ret = *ptr;
  do {
    old = ret;
    newdbl = __double_as_longlong(__longlong_as_double(old)+val);
  } while((ret = atomicCAS(ptr, old, newdbl)) != old);
  return __longlong_as_double(ret);
}

template <typename T>
void atomicMinDerived (T *res)
{
  int i = _tid_x + _bid_x * BLOCK_DIM_X + 1;
  atomic_min(res, (T)i);
}

template <typename T>
void atomicMaxDerived (T *res)
{
  int i = _tid_x + _bid_x * BLOCK_DIM_X + 1;
  atomic_max(res, (T)i);
}

template <typename T>
void atomicAddDerived (T *res)
{
  int i = _tid_x + _bid_x * BLOCK_DIM_X + 1;
  atomic_add(res, (T)i);
}
