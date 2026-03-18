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
#ifndef BLOCK_DIM_Z
#define BLOCK_DIM_Z 1
#endif
#ifndef GRID_DIM_Z
#define GRID_DIM_Z 1
#endif

// --- from kernels.cu ---



}





// --- from main.cu ---
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>

/*===========================================================================*/
/*---Main---*/



// --- from utils.cu ---

// "* 1" removes the warning:
// enumeral mismatch in conditional expression: ‘<anonymous enum>’ vs ‘<anonymous enum>’


/*===========================================================================*/
/* Process an argument of type int, remove from list---*/


/*===========================================================================*/
/* Consume an argument of type int, if not present then set to a default---*/


/*===========================================================================*/
/* Pseudo-destructor for Arguments struct---*/








/*---------------------------------------------------------------------------*/







/*---Size of state vector in angles space---*/



// --- from utils.h ---
#ifndef _UTILS
#define _UTILS

#define P double

/*---Number of physical dimensions---*/
enum{ NDIM = 3 };
enum{ DIM_X = 0 };
enum{ DIM_Y = 1 };
enum{ DIM_Z = 2 };

/*---Number of octant directions---*/
enum{ NOCTANT = 8 };

/*---Enums, functions to manipulate sweep directions---*/
enum{ DIR_UP = +1 }; /*---up or down---*/
enum{ DIR_DN = -1 };

enum{ DIR_HI = +1 }; /*---high side or low side---*/
enum{ DIR_LO = -1 };

typedef struct
{
  int    argc;
  char** argv_unconsumed; /*---Working copy of argument list---*/
  char*  argstring;
} Arguments;

typedef struct
{
  int nblock_z_;
  int nproc_x_;
  int nproc_y_;
  int nblock_octant_;
  int noctant_per_block_;
} StepScheduler;

/*---Number of unknowns per gridcell, moment, energy group---*/
#ifdef NU_VALUE
enum{ NU = NU_VALUE };
#else
enum{ NU = 4 }; /*---DEFAULT---*/
#endif

/*---Number of moments---*/
#ifdef NM_VALUE
enum{ NM = NM_VALUE };
#else
enum{ NM = 4 }; /*---DEFAULT---*/
#endif

/*===========================================================================*/
/*---Struct to hold problem dimensions---*/

typedef struct
{
  /*---Grid spatial dimensions---*/
  int ncell_x;
  int ncell_y;
  int ncell_z;

  /*---Number of energy groups---*/
  int ne;

  /*----Number of moments---*/
  int nm;

  /*---Number of angles---*/
  int na;
} Dimensions;

typedef struct
{
  //P*   facexy;
  //P*   facexz;
  //P*   faceyz;
  P*   vslocal_host_;

  int              nblock_z;
  int              nblock_octant;
  int              noctant_per_block;

  Dimensions       dims;
  Dimensions       dims_b;

  StepScheduler    stepscheduler;

  //Faces            faces;
} Sweeper;

/*===========================================================================*/
/*---Struct with info describing a sweep step---*/

typedef struct
{
  int     block_z;
  int     octant;
  int     is_active;
} StepInfo;

/*===========================================================================*/
/*---8 copies of the same---*/

typedef struct
{
  StepInfo stepinfo[NOCTANT];
} StepInfoAll;

#define A_FROM_M_ADDR(dims_na, im, ia, octant) \
  (ia) + (dims_na) * ((im) + NM * octant)

#define M_FROM_A_ADDR(dims_na, im, ia, octant) \
  (im) + NM * ((ia) + (dims_na)  * octant)

#define FACEXY_ADDR(x, y) \
  ia + dims_b_na * (iu + NU * ( ie + dims_b_ne * (ix + x * (iy + y * octant))))

#define FACEXZ_ADDR(x, z) \
  ia + dims_b_na * (iu + NU * ( ie + dims_b_ne * (ix + x * (iz + z * octant))))

#define FACEYZ_ADDR(y, z) \
  ia + dims_b_na * (iu + NU * ( ie + dims_b_ne * (iy + y * (iz + z * octant))))

#endif 
