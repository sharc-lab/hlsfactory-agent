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

// --- from compress_kernel.cu ---
// statistical kernel


// --- from extract_kernel.cu ---
// statistical kernel


// --- from main.cu ---
//====================================================================================================100
//    UPDATE
//====================================================================================================100

//    2006.03   Rob Janiczek
//        --creation of prototype version
//    2006.03   Drew Gilliam
//        --rewriting of prototype version into current version
//        --got rid of multiple function calls, all code in a  
//         single function (for speed)
//        --code cleanup & commenting
//        --code optimization efforts   
//    2006.04   Drew Gilliam
//        --added diffusion coefficent saturation on [0,1]
//    2009.12 Lukasz G. Szafaryn
//    -- reading from image, command line inputs
//    2010.01 Lukasz G. Szafaryn
//    --comments

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "graphics.c"
#include "resize.c"
#include "timer.c"



// --- from prepare_kernel.cu ---
// statistical kernel


// --- from reduce_kernel.cu ---
// statistical kernel


// --- from srad2_kernel.cu ---
// srad kernel


// --- from srad_kernel.cu ---
// srad kernel


// --- from include.h ---
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include <mex.h>

#include <define.h>

#include <print.c>

#include <prepare_kernel.cu>
#include <extract_kernel.cu>
#include <reduce_kernel.cu>
#include <srad_kernel.cu>
#include <srad2_kernel.cu>
#include <compress_kernel.cu>

// --- from main.h ---
//====================================================================================================100
//====================================================================================================100
//	DEFINE
//====================================================================================================100
//====================================================================================================100

#define fp float

#define NUMBER_THREADS 256
