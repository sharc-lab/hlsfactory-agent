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
/* This example is a very small one designed to show how compact SYCL code
 * can be. That said, it includes no error checking and is rather terse. */
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <chrono>
#include <cmath>

const float GDC_DEG_TO_RAD = 3.141592654 / 180.0 ;  /* Degrees to radians */
const float GDC_FLATTENING = 1.0 - ( 6356752.31424518 / 6378137.0 ) ;
const float GDC_ECCENTRICITY = ( 6356752.31424518 / 6378137.0 ) ;
const float GDC_ELLIPSOIDAL =  1.0 / ( 6356752.31414 / 6378137.0 ) / ( 6356752.31414 / 6378137.0 ) - 1.0 ;
const float GC_SEMI_MINOR = 6356752.31424518f;
const float EPS = 0.5e-5f;





