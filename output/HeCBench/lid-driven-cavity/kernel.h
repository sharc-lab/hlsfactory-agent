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
/** GPU solver for 2D lid-driven cavity problem, using finite difference method
 * \file main_gpu.cu
 *
 * \author Kyle E. Niemeyer
 * \date 09/27/2012
 *
 * Solve the incompressible, isothermal 2D Navier–Stokes equations for a square
 * lid-driven cavity on a GPU (via CUDA), using the finite difference method.
 * To change the grid resolution, modify "NUM". In addition, the problem is controlled
 * by the Reynolds number ("Re_num").
 * 
 * Based on the methodology given in Chapter 3 of "Numerical Simulation in Fluid
 * Dynamics", by M. Griebel, T. Dornseifer, and T. Neunhoeffer. SIAM, Philadelphia,
 * PA, 1998.
 * 
 * Boundary conditions:
 * u = 0 and v = 0 at x = 0, x = L, y = 0
 * u = ustar at y = H
 * v = 0 at y = H
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <chrono>

/** Problem size along one side; total number of cells is this squared */
#define NUM 512

// block size
#define BLOCK_SIZE 128

/** Double precision */
#define DOUBLE

#ifdef DOUBLE
#define Real double

#define ZERO 0.0
#define ONE 1.0
#define TWO 2.0
#define FOUR 4.0

#define SMALL 1.0e-10;

/** Reynolds number */
const Real Re_num = 1000.0;

/** SOR relaxation parameter */
const Real omega = 1.7;

/** Discretization mixture parameter (gamma) */
const Real mix_param = 0.9;

/** Safety factor for time step modification */
const Real tau = 0.5;

/** Body forces in x- and y- directions */
const Real gx = 0.0;
const Real gy = 0.0;

/** Domain size (non-dimensional) */
#define xLength 1.0
#define yLength 1.0
#else
#define Real float

// replace double functions with float versions
#undef fmin
#define fmin fminf
#undef fmax
#define fmax fmaxf
#undef fabs
#define fabs fabsf
#undef sqrt
#define sqrt sqrtf

#define ZERO 0.0f
#define ONE 1.0f
#define TWO 2.0f
#define FOUR 4.0f
#define SMALL 1.0e-10f;

/** Reynolds number */
const Real Re_num = 1000.0f;

/** SOR relaxation parameter */
const Real omega = 1.7f;

/** Discretization mixture parameter (gamma) */
const Real mix_param = 0.9f;

/** Safety factor for time step modification */
const Real tau = 0.5f;

/** Body forces in x- and y- directions */
const Real gx = 0.0f;
const Real gy = 0.0f;

/** Domain size (non-dimensional) */
#define xLength 1.0f
#define yLength 1.0f
#endif

/** Mesh sizes */
const Real dx = xLength / NUM;
const Real dy = yLength / NUM;

/** Max macro (type safe, from GNU) */
//#define MAX(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })

/** Min macro (type safe) */
//#define MIN(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })

// map two-dimensional indices to one-dimensional memory
#define u(I, J) u[((I) * ((NUM) + 2)) + (J)]
#define v(I, J) v[((I) * ((NUM) + 2)) + (J)]
#define F(I, J) F[((I) * ((NUM) + 2)) + (J)]
#define G(I, J) G[((I) * ((NUM) + 2)) + (J)]
#define pres_red(I, J) pres_red[((I) * ((NUM_2) + 2)) + (J)]
#define pres_black(I, J) pres_black[((I) * ((NUM_2) + 2)) + (J)]

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

/** Function to update pressure for red cells
 * 
 * \param[in]    dt      time-step size
 * \param[in]    F      array of discretized x-momentum eqn terms
 * \param[in]    G      array of discretized y-momentum eqn terms
 * \param[in]    pres_black  pressure values of black cells
 * \param[inout]  pres_red  pressure values of red cells
 */

///////////////////////////////////////////////////////////////////////////////

/** Function to update pressure for black cells
 * 
 * \param[in]    dt      time-step size
 * \param[in]    F      array of discretized x-momentum eqn terms
 * \param[in]    G      array of discretized y-momentum eqn terms
 * \param[in]    pres_red  pressure values of red cells
 * \param[inout]  pres_black  pressure values of black cells
 */

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

