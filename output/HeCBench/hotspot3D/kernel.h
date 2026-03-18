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

// --- from 3D.cu ---
#include <sys/types.h>
#include <chrono>

#define TOL      (0.001)
#define STR_SIZE (256)
#define MAX_PD   (3.0e6)

/* required precision in degrees  */
#define PRECISION    0.001
#define SPEC_HEAT_SI 1.75e6
#define K_SI         100

/* capacitance fitting factor  */
#define FACTOR_CHIP  0.5

#define WG_SIZE_X (64)
#define WG_SIZE_Y (4)
float t_chip      = 0.0005;
float chip_height = 0.016;
float chip_width  = 0.016;
float amb_temp    = 80.0;





// --- from 3D_helper.cu ---

#define STR_SIZE 256








// --- from 3D_helper.h ---
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

void  fatal(const char* s);
void  readinput(float* v, int r, int c,int l,char*);
void  writeoutput(float* v,int r,int c,int l,char*);
long long get_time(); 
float accuracy(float* arr1, float* arr2, int len);
void computeTempCPU(float* pIn, float *tIn, float *tOut, 
               int nx, int ny, int nz, float Cap,
               float Rx, float Ry, float Rz, 
               float dt, float amb_temp, int numiter); 
