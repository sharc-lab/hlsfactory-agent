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
// ----------------------------------------------------------------------
// Copyright (2019) Sandia Corporation.
// Under the terms of Contract DE-AC04-94AL85000
// with Sandia Corporation, the U.S. Government
// retains certain rights in this software. This
// software is distributed under the Zero Clause
// BSD License
//
// TestSNAP - A prototype for the SNAP force kernel
// Version 0.0.2
// Main changes: Y array trick, memory compaction
//
// Original author: Aidan P. Thompson, athomps@sandia.gov
// http://www.cs.sandia.gov/~athomps, Sandia National Laboratories
//
// ----------------------------------------------------------------------
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <hip/hip_runtime.h>

#if REFDATA_TWOJ == 14
#include "refdata_2J14_W.h"
#elif REFDATA_TWOJ == 8
#include "refdata_2J8_W.h"
#elif REFDATA_TWOJ == 4
#include "refdata_2J4_W.h"
#else
#include "refdata_2J2_W.h"
#endif

int nsteps = 1; // num of force evaluations










// --- from utils.cu ---
// ----------------------------------------------------------------------
// Copyright (2019) Sandia Corporation.
// Under the terms of Contract DE-AC04-94AL85000
// with Sandia Corporation, the U.S. Government
// retains certain rights in this software. This
// software is distributed under the Zero Clause
// BSD License
//
// TestSNAP - A prototype for the SNAP force kernel
// Version 0.0.2
// Main changes: Y array trick, memory compaction
//
// Original author: Aidan P. Thompson, athomps@sandia.gov
// http://www.cs.sandia.gov/~athomps, Sandia National Laboratories
//
// ----------------------------------------------------------------------

/* ----------------------------------------------------------------------

   this implementation is based on the method outlined
   in Bartok[1], using formulae from VMK[2].

   for the Clebsch-Gordan coefficients, we
   convert the VMK half-integral labels
   a, b, c, alpha, beta, gamma
   to array offsets j1, j2, j, m1, m2, m
   using the following relations:

   j1 = 2*a
   j2 = 2*b
   j =  2*c

   m1 = alpha+a      2*alpha = 2*m1 - j1
   m2 = beta+b    or 2*beta = 2*m2 - j2
   m =  gamma+c      2*gamma = 2*m - j

   in this way:

   -a <= alpha <= a
   -b <= beta <= b
   -c <= gamma <= c

   becomes:

   0 <= m1 <= j1
   0 <= m2 <= j2
   0 <= m <= j

   and the requirement that
   a+b+c be integral implies that
   j1+j2+j must be even.
   The requirement that:

   gamma = alpha+beta

   becomes:

   2*m - j = 2*m1 - j1 + 2*m2 - j2

   Similarly, for the Wigner U-functions U(J,m,m') we
   convert the half-integral labels J,m,m' to
   array offsets j,ma,mb:

   j = 2*J
   ma = J+m
   mb = J+m'

   so that:

   0 <= j <= 2*Jmax
   0 <= ma, mb <= j.

   For the bispectrum components B(J1,J2,J) we convert to:

   j1 = 2*J1
   j2 = 2*J2
   j = 2*J

   and the requirement:

   |J1-J2| <= J <= J1+J2, for j1+j2+j integral

   becomes:

   |j1-j2| <= j <= j1+j2, for j1+j2+j even integer

   or

   j = |j1-j2|, |j1-j2|+2,...,j1+j2-2,j1+j2

   [1] Albert Bartok-Partay, "Gaussian Approximation..."
   Doctoral Thesis, Cambrindge University, (2009)

   [2] D. A. Varshalovich, A. N. Moskalev, and V. K. Khersonskii,
   "Quantum Theory of Angular Momentum," World Scientific (1988)

------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
   factorial n table, size SNA::nmaxfactorial+1
------------------------------------------------------------------------- */

const double nfac_table[] = {
  1,
  1,
  2,
  6,
  24,
  120,
  720,
  5040,
  40320,
  362880,
  3628800,
  39916800,
  479001600,
  6227020800,
  87178291200,
  1307674368000,
  20922789888000,
  355687428096000,
  6.402373705728e+15,
  1.21645100408832e+17,
  2.43290200817664e+18,
  5.10909421717094e+19,
  1.12400072777761e+21,
  2.5852016738885e+22,
  6.20448401733239e+23,
  1.5511210043331e+25,
  4.03291461126606e+26,
  1.08888694504184e+28,
  3.04888344611714e+29,
  8.8417619937397e+30,
  2.65252859812191e+32,
  8.22283865417792e+33,
  2.63130836933694e+35,
  8.68331761881189e+36,
  2.95232799039604e+38,
  1.03331479663861e+40,
  3.71993326789901e+41,
  1.37637530912263e+43,
  5.23022617466601e+44,
  2.03978820811974e+46, // nmaxfactorial = 39
};

/* ----------------------------------------------------------------------
   the function delta given by VMK Eq. 8.2(1)
------------------------------------------------------------------------- */


/* ----------------------------------------------------------------------
   assign Clebsch-Gordan coefficients using
   the quasi-binomial formula VMK 8.2.1(3)
------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
   pre-compute table of sqrt[p/m2], p, q = 1,twojmax
   the p = 0, q = 0 entries are allocated and skipped for convenience.
   a second table is computed with +/-1 parity factor
------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */


/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */


/* ----------------------------------------------------------------------
  Elapsed Time
------------------------------------------------------------------------- */


/* ----------------------------------------------------------------------
   factorial n, wrapper for precomputed table
------------------------------------------------------------------------- */




// --- from snap.h ---

#ifndef SNAP_H
#define SNAP_H

#include <chrono>

extern int nsteps;

using namespace std::chrono;
typedef high_resolution_clock myclock;
typedef duration<float> myduration;

// Complex type structure
typedef struct
{
  double re;
  double im;
} COMPLEX;

typedef struct 
{
  int j1; 
  int j2;
  int j;
} SNA_BINDICES;

double factorial(int);
double deltacg(int, int, int);
int compute_ncoeff(int twojmax);
void options(int argc, char* argv[]);
inline double elapsedTime(timeval start_time, timeval end_time);

double compute_sfac(double, double, int);
double compute_dsfac(double, double, int);
void compute_duarray(
    const int natom,
    const int nbor,
    const int num_atoms,
    const int num_nbor,
    const int twojmax,
    const int idxdu_max,
    const int jdimpq,
    const int switch_flag,
    const double x,
    const double y,
    const double z,
    const double z0,
    const double r,
    const double dz0dr,
    const double wj_in,
    const double rcut,
    const double* rootpqarray,
    const COMPLEX* ulist,
    COMPLEX* dulist);

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MY_PI  3.14159265358979323846
#define nmaxfactorial 167
#define rfac0  0.99363
#define rmin0  0.0

#define  DULIST_INDEX(a, b, c, d) \
  ((a) + num_atoms * (b) + num_atoms * num_nbor * (c) + num_atoms * num_nbor * idxdu_max * (d))

#define  ULIST_INDEX(a, b, c) \
  ((a) + num_atoms * (b) + num_atoms * num_nbor * (c))

#define ROOTPQ_INDEX(a, b) ((a) + jdimpq * (b))

#define  INDEX_2D(a, b) ((a) + num_atoms * (b))

#define  F_INDEX(a, b) ((a) + ntotal * (b))

#define  IDXZ_INDEX(a, b) ((a) + idxz_max * (b))

#endif
