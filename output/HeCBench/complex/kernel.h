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
#include <chrono>




// --- from complex.h ---
#if !defined(COMPLEX_H_)
#define COMPLEX_H_

#include <math.h>       /* import fabsf, sqrt */

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

typedef float2 FloatComplex;

static __inline__ float Crealf (FloatComplex x) 
{ 
    return x.x; 
}

static __inline__ float Cimagf (FloatComplex x) 
{ 
    return x.y; 
}

static __inline__ FloatComplex make_FloatComplex (float r, float i)
{
    FloatComplex res;
    res.x = r;
    res.y = i;
    return res;
}

static __inline__ FloatComplex Conjf (FloatComplex x)
{
    return make_FloatComplex (Crealf(x), -Cimagf(x));
}

static __inline__ FloatComplex Caddf (FloatComplex x, FloatComplex y)
{
    return make_FloatComplex (Crealf(x) + Crealf(y), 
                                Cimagf(x) + Cimagf(y));
}

static __inline__ FloatComplex Csubf (FloatComplex x, FloatComplex y)
{
    return make_FloatComplex (Crealf(x) - Crealf(y), 
                                    Cimagf(x) - Cimagf(y));
}

/* This implementation could suffer from intermediate overflow even though
 * the final result would be in range. However, various implementations do
 * not guard against this (presumably to avoid losing performance), so we 
 * don't do it either to stay competitive.
 */
static __inline__ FloatComplex Cmulf (FloatComplex x, FloatComplex y)
{
    FloatComplex prod;
    prod = make_FloatComplex  ((Crealf(x) * Crealf(y)) - 
                                 (Cimagf(x) * Cimagf(y)),
                                 (Crealf(x) * Cimagf(y)) + 
                                 (Cimagf(x) * Crealf(y)));
    return prod;
}

/* This implementation guards against intermediate underflow and overflow
 * by scaling. Such guarded implementations are usually the default for
 * complex library implementations, with some also offering an unguarded,
 * faster version.
 */
static __inline__ FloatComplex Cdivf (FloatComplex x, FloatComplex y)
{
    FloatComplex quot;
    float s = fabsf(Crealf(y)) + fabsf(Cimagf(y));
    float oos = 1.0f / s;
    float ars = Crealf(x) * oos;
    float ais = Cimagf(x) * oos;
    float brs = Crealf(y) * oos;
    float bis = Cimagf(y) * oos;
    s = (brs * brs) + (bis * bis);
    oos = 1.0f / s;
    quot = make_FloatComplex (((ars * brs) + (ais * bis)) * oos,
                                ((ais * brs) - (ars * bis)) * oos);
    return quot;
}

/* 
 * We would like to call hypotf(), but it's not available on all platforms.
 * This discrete implementation guards against intermediate underflow and 
 * overflow by scaling. Otherwise we would lose half the exponent range. 
 * There are various ways of doing guarded computation. For now chose the 
 * simplest and fastest solution, however this may suffer from inacracies 
 * if sqrt and division are not IEEE compliant. 
 */
static __inline__ float Cabsf (FloatComplex x)
{
    float a = Crealf(x);
    float b = Cimagf(x);
    float v, w, t;
    a = fabsf(a);
    b = fabsf(b);
    if (a > b) {
        v = a;
        w = b; 
    } else {
        v = b;
        w = a;
    }
    t = w / v;
    t = 1.0f + t * t;
    t = v * sqrtf(t);
    if ((v == 0.0f) || (v > 3.402823466e38f) || (w > 3.402823466e38f)) {
        t = v + w;
    }
    return t;
}

/* Double precision */
typedef double2 DoubleComplex;

static __inline__ double Creal (DoubleComplex x) 
{ 
    return x.x; 
}

static __inline__ double Cimag (DoubleComplex x) 
{ 
    return x.y; 
}

static __inline__ DoubleComplex make_DoubleComplex (double r, double i)
{
    DoubleComplex res;
    res.x = r;
    res.y = i;
    return res;
}

static __inline__ DoubleComplex Conj(DoubleComplex x)
{
    return make_DoubleComplex (Creal(x), -Cimag(x));
}

static __inline__ DoubleComplex Cadd(DoubleComplex x, DoubleComplex y)
{
    return make_DoubleComplex (Creal(x) + Creal(y), 
                                 Cimag(x) + Cimag(y));
}

static __inline__ DoubleComplex Csub(DoubleComplex x, DoubleComplex y)
{
    return make_DoubleComplex (Creal(x) - Creal(y), 
                                 Cimag(x) - Cimag(y));
}

/* This implementation could suffer from intermediate overflow even though
 * the final result would be in range. However, various implementations do
 * not guard against this (presumably to avoid losing performance), so we 
 * don't do it either to stay competitive.
 */
static __inline__ DoubleComplex Cmul(DoubleComplex x, DoubleComplex y)
{
    DoubleComplex prod;
    prod = make_DoubleComplex ((Creal(x) * Creal(y)) - 
                                 (Cimag(x) * Cimag(y)),
                                 (Creal(x) * Cimag(y)) + 
                                 (Cimag(x) * Creal(y)));
    return prod;
}

/* This implementation guards against intermediate underflow and overflow
 * by scaling. Such guarded implementations are usually the default for
 * complex library implementations, with some also offering an unguarded,
 * faster version.
 */
static __inline__ DoubleComplex Cdiv(DoubleComplex x, DoubleComplex y)
{
    DoubleComplex quot;
    double s = (fabs(Creal(y))) + (fabs(Cimag(y)));
    double oos = 1.0 / s;
    double ars = Creal(x) * oos;
    double ais = Cimag(x) * oos;
    double brs = Creal(y) * oos;
    double bis = Cimag(y) * oos;
    s = (brs * brs) + (bis * bis);
    oos = 1.0 / s;
    quot = make_DoubleComplex (((ars * brs) + (ais * bis)) * oos,
                                 ((ais * brs) - (ars * bis)) * oos);
    return quot;
}

/* This implementation guards against intermediate underflow and overflow
 * by scaling. Otherwise we would lose half the exponent range. There are
 * various ways of doing guarded computation. For now chose the simplest
 * and fastest solution, however this may suffer from inacracies if sqrt
 * and division are not IEEE compliant.
 */
static __inline__ double Cabs (DoubleComplex x)
{
    double a = Creal(x);
    double b = Cimag(x);
    double v, w, t;
    a = fabs(a);
    b = fabs(b);
    if (a > b) {
        v = a;
        w = b; 
    } else {
        v = b;
        w = a;
    }
    t = w / v;
    t = 1.0 + t * t;
    t = v * sqrt(t);
    if ((v == 0.0) || 
        (v > 1.79769313486231570e+308) || (w > 1.79769313486231570e+308)) {
        t = v + w;
    }
    return t;
}

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/* aliases */
typedef FloatComplex Complex;
static __inline__ Complex make_Complex (float x, float y) 
{ 
    return make_FloatComplex (x, y); 
}

/* float-to-double promotion */
static __inline__ DoubleComplex ComplexFloatToDouble (FloatComplex c)
{
    return make_DoubleComplex ((double)Crealf(c), (double)Cimagf(c));
}

static __inline__ FloatComplex ComplexDoubleToFloat (DoubleComplex c)
{
    return make_FloatComplex ((float)Creal(c), (float)Cimag(c));
}

static __inline__  Complex Cfmaf( Complex x, Complex y, Complex d)
{
    float real_res;
    float imag_res;
    
    real_res = (Crealf(x) *  Crealf(y)) + Crealf(d);
    imag_res = (Crealf(x) *  Cimagf(y)) + Cimagf(d);
            
    real_res = -(Cimagf(x) * Cimagf(y))  + real_res;  
    imag_res =  (Cimagf(x) *  Crealf(y)) + imag_res;          
     
    return make_Complex(real_res, imag_res);
}

static __inline__  DoubleComplex Cfma( DoubleComplex x, DoubleComplex y, DoubleComplex d)
{
    double real_res;
    double imag_res;
    
    real_res = (Creal(x) *  Creal(y)) + Creal(d);
    imag_res = (Creal(x) *  Cimag(y)) + Cimag(d);
            
    real_res = -(Cimag(x) * Cimag(y))  + real_res;  
    imag_res =  (Cimag(x) *  Creal(y)) + imag_res;     
     
    return make_DoubleComplex(real_res, imag_res);
}

#endif /* !defined(COMPLEX_H_) */


// --- from kernels.h ---

double LCG_random_double(uint64_t * seed)
{
  const uint64_t m = 9223372036854775808ULL; // 2^63
  const uint64_t a = 2806196910506780709ULL;
  const uint64_t c = 1ULL;
  *seed = (a * (*seed) + c) % m;
  return (double) (*seed) / (double) m;
}

uint64_t fast_forward_LCG(uint64_t seed, uint64_t n)
{
  const uint64_t m = 9223372036854775808ULL; // 2^63
  uint64_t a = 2806196910506780709ULL;
  uint64_t c = 1ULL;

  n = n % m;

  uint64_t a_new = 1;
  uint64_t c_new = 0;

  while(n > 0) 
  {
    if(n & 1)
    {
      a_new *= a;
      c_new = c_new * a + c;
    }
    c *= (a + 1);
    a *= a;

    n >>= 1;
  }

  return (a_new * seed + c_new) % m;
}

void complex_float (char* checkSum, int n)
{
  int i = _tid_x + _bid_x * BLOCK_DIM_X;
  if (i >= n) return; 
  uint64_t seed = 1ULL;
  seed = fast_forward_LCG(seed, i);
  float r1 = LCG_random_double(&seed);
  float r2 = LCG_random_double(&seed); 
  float r3 = LCG_random_double(&seed); 
  float r4 = LCG_random_double(&seed); 

  FloatComplex z1 = make_FloatComplex(r1, r2);
  FloatComplex z2 = make_FloatComplex(r3, r4);

  char s = fabsf(Cabsf(Cmulf(z1, z2)) - Cabsf(z1) * Cabsf(z2)) < 1e-3f;

  s += fabsf(Cabsf(Caddf(z1, z2)) * Cabsf(Caddf(z1 , z2)) -
             Crealf(Cmulf(Caddf(z1, z2) , Caddf(Conjf(z1), Conjf(z2))))) < 1e-3f; 

  s += fabsf(Cabsf(Csubf(z1, z2)) * Cabsf(Csubf(z1 , z2)) -
             Crealf(Cmulf(Csubf(z1, z2) , Csubf(Conjf(z1), Conjf(z2))))) < 1e-3f;

  s += fabsf(Crealf(Caddf(Cmulf(z1, Conjf(z2)) , Cmulf(z2, Conjf(z1)))) -
             2.0f * (Crealf(z1) * Crealf(z2) + Cimagf(z1) * Cimagf(z2))) < 1e-3f;

  s += fabsf(Cabsf(Cdivf(Conjf(z1), z2)) -
             Cabsf(Cdivf(Conjf(z1), Conjf(z2)))) < 1e-3f;

  checkSum[i] = s;
}

void complex_double (char* checkSum, int n)
{
  int i = _tid_x + _bid_x * BLOCK_DIM_X;
  if (i >= n) return; 
  uint64_t seed = 1ULL;
  seed = fast_forward_LCG(seed, i);
  double r1 = LCG_random_double(&seed);
  double r2 = LCG_random_double(&seed); 
  double r3 = LCG_random_double(&seed); 
  double r4 = LCG_random_double(&seed); 

  DoubleComplex z1 = make_DoubleComplex(r1, r2);
  DoubleComplex z2 = make_DoubleComplex(r3, r4);

  char s = fabs(Cabs(Cmul(z1, z2)) - Cabs(z1) * Cabs(z2)) < 1e-3;

  s += fabs(Cabs(Cadd(z1, z2)) * Cabs(Cadd(z1 , z2)) -
            Creal(Cmul(Cadd(z1, z2) , Cadd(Conj(z1), Conj(z2))))) < 1e-3; 

  s += fabs(Cabs(Csub(z1, z2)) * Cabs(Csub(z1 , z2)) -
            Creal(Cmul(Csub(z1, z2) , Csub(Conj(z1), Conj(z2))))) < 1e-3;

  s += fabs(Creal(Cadd(Cmul(z1, Conj(z2)) , Cmul(z2, Conj(z1)))) -
            2.0 * (Creal(z1) * Creal(z2) + Cimag(z1) * Cimag(z2))) < 1e-3;

  s += fabs(Cabs(Cdiv(Conj(z1), z2)) -
            Cabs(Cdiv(Conj(z1), Conj(z2)))) < 1e-3;

  checkSum[i] = s;
}

