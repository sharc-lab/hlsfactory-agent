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
#include <iostream>
#include <cstdio>
#include <chrono>

#define NOW std::chrono::high_resolution_clock::now()


// Functional test returns 1 when it fails; otherwise it returns 0



// --- from fastdiv.h ---
#ifndef _INT_FASTDIV_
#define _INT_FASTDIV_

#if (defined(__CUDACC__) || defined (__HIPCC__))
 #define ESS inline
#else
 #define ESS inline
#endif

class int_fastdiv
{
  public:
    // divisor != 0 
    ESS 
    int_fastdiv(int divisor = 0) : d(divisor)
    {
      update_magic_numbers();
    }

    ESS
    int_fastdiv& operator= (int divisor)
    {
      this->d = divisor;
      update_magic_numbers();
      return *this;
    }

    ESS
    operator int() const
    {
      return d;
    }

  private:
    int d;
    int M;
    int s;
    int n_add_sign;

    // Hacker's Delight, Second Edition, Chapter 10, Integer Division By Constants
    ESS
    void update_magic_numbers()
    {
      if (d == 1)
      {
        M = 0;
        s = -1;
        n_add_sign = 1;
        return;
      }
      else if (d == -1)
      {
        M = 0;
        s = -1;
        n_add_sign = -1;
        return;
      }

      int p;
      unsigned int ad, anc, delta, q1, r1, q2, r2, t;
      const unsigned two31 = 0x80000000;
      if (d < 0) 
        ad = -d;
      else if (d == 0)
        ad = 1;
      else 
        ad = d;
      t = two31 + ((unsigned int)d >> 31);
      anc = t - 1 - t % ad;
      p = 31;
      q1 = two31 / anc;
      r1 = two31 - q1 * anc;
      q2 = two31 / ad;
      r2 = two31 - q2 * ad;
      do
      {
        ++p;
        q1 = 2 * q1;
        r1 = 2 * r1;
        if (r1 >= anc)
        {
          ++q1;
          r1 -= anc;
        }
        q2 = 2 * q2;
        r2 = 2 * r2;
        if (r2 >= ad)
        {
          ++q2;
          r2 -= ad;
        }
        delta = ad - r2;
      } while (q1 < delta || (q1 == delta && r1 == 0));
      this->M = q2 + 1;
      if (d < 0)
        this->M = -this->M;
      this->s = p - 32;

      if ((d > 0) && (M < 0))
        n_add_sign = 1;
      else if ((d < 0) && (M > 0))
        n_add_sign = -1;
      else
        n_add_sign = 0;      
    }

    ESS
    friend int operator/(const int divident, const int_fastdiv& divisor);
};

ESS
int operator/(const int n, const int_fastdiv& divisor)
{
  int q;
  q = (((unsigned long long)((long long)divisor.M * (long long)n)) >> 32);
  q += n * divisor.n_add_sign;
  if (divisor.s >= 0)
  {
    q >>= divisor.s; // we rely on this to be implemented as arithmetic shift
    q += (((unsigned int)q) >> 31);
  }
  return q;
}

ESS
int operator%(const int n, const int_fastdiv& divisor)
{
  int quotient = n / divisor;
  int remainder = n - quotient * divisor;
  return remainder;
}

ESS
int operator/(const unsigned int n, const int_fastdiv& divisor)
{
  return ((int)n) / divisor;
}

ESS
int operator%(const unsigned int n, const int_fastdiv& divisor)
{
  return ((int)n) % divisor;
}

ESS
int operator/(const short n, const int_fastdiv& divisor)
{
  return ((int)n) / divisor;
}

ESS
int operator%(const short n, const int_fastdiv& divisor)
{
  return ((int)n) % divisor;
}

ESS
int operator/(const unsigned short n, const int_fastdiv& divisor)
{
  return ((int)n) / divisor;
}

ESS
int operator%(const unsigned short n, const int_fastdiv& divisor)
{
  return ((int)n) % divisor;
}

ESS
int operator/(const char n, const int_fastdiv& divisor)
{
  return ((int)n) / divisor;
}

ESS
int operator%(const char n, const int_fastdiv& divisor)
{
  return ((int)n) % divisor;
}

ESS
int operator/(const unsigned char n, const int_fastdiv& divisor)
{
  return ((int)n) / divisor;
}

ESS
int operator%(const unsigned char n, const int_fastdiv& divisor)
{
  return ((int)n) % divisor;
}

#endif


// --- from kernels.h ---
template<typename divisor_type>
void throughput_test(
    divisor_type d1,
    divisor_type d2,
    divisor_type d3,
    int dummy,
    int * buf)
{
  int x = _bid_x * BLOCK_DIM_X + _tid_x;
  int x1 = x / d1;
  int x2 = x / d2;
  int x3 = x / d3;
  int aggregate = x1 + x2 + x3;  
  if (aggregate && dummy) buf[0] = aggregate;
}

template<typename divisor_type>
void latency_test(
    divisor_type d1,
    divisor_type d2,
    divisor_type d3,
    divisor_type d4,
    divisor_type d5,
    divisor_type d6,
    divisor_type d7,
    divisor_type d8,
    divisor_type d9,
    divisor_type d10,
    int dummy,
    int * buf)
{
  int x = _bid_x * BLOCK_DIM_X + _tid_x;
  x /= d1;
  x /= d2;
  x /= d3;
  x /= d4;
  x /= d5;
  x /= d6;
  x /= d7;
  x /= d8;
  x /= d9;
  x /= d10;
  if (x && dummy) buf[0] = x;
}

void check(int_fastdiv divisor, int * results)
{
  int divident = _bid_x * BLOCK_DIM_X + _tid_x;

  int quotient = divident / (int)divisor;
  int fast_quotient = divident / divisor;

  if (quotient != fast_quotient)
  {
    int error_id = (results[0] += 1);
    if (error_id == 0)
    {
      results[1] = divident;
      results[2] = quotient;
      results[3] = fast_quotient;
    }
  }

  divident = -divident;
  quotient = divident / (int)divisor;
  fast_quotient = divident / divisor;

  if (quotient != fast_quotient)
  {
    int error_id = (results[0] += 1);
    if (error_id == 0)
    {
      results[1] = divident;
      results[2] = quotient;
      results[3] = fast_quotient;
    }
  }
}
