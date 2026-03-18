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

// --- from kernels.cu ---
/*
  Copyright 2015  Hung-Yi Pu, Kiyun Yun, Ziri Younsi, Sunk-Jin Yoon
  Odyssey  version 1.0   (released  2015)
  This file is part of Odyssey source code. Odyssey is a public, GPU-based code 





//
// functions for radiative transfer                                                  
//

#define Te_min    0.1
#define Te_max    100.
#define Te_grids  50.

static double K2_tab[] = {
  -10.747001, //Te=0.1
  -9.362569,
  -8.141373,
  -7.061568,
  -6.104060,
  -5.252153,
  -4.491244,
  -3.808555,
  -3.192909,
  -2.634534,
  -2.124893,
  -1.656543,
  -1.223007,
  -0.818668,
  -0.438676,
  -0.078863,
  +0.264332,
  +0.593930,
  +0.912476,
  +1.222098,
  +1.524560,
  +1.821311,
  +2.113537,
  +2.402193,
  +2.688050,
  +2.971721,
  +3.253692,
  +3.534347,
  +3.813984,
  +4.092839,
  +4.371092,
  +4.648884,
  +4.926323,
  +5.203493,
  +5.480457,
  +5.757264,
  +6.033952,
  +6.310550,
  +6.587078,
  +6.863554,
  +7.139990,
  +7.416395,
  +7.692778,
  +7.969143,
  +8.245495,
  +8.521837,
  +8.798171,
  +9.074500,
  +9.350824,
  +9.627144  //Te=87.09
};










// --- from main.cu ---
/***********************************************************************************
  Copyright 2015  Hung-Yi Pu, Kiyun Yun, Ziri Younsi, Sunk-Jin Yoon
  Odyssey  version 1.0   (released  2015)
  This file is part of Odyssey source code. Odyssey is a public, GPU-based code 
  for General Relativistic Radiative Transfer (GRRT), following the 
  ray-tracing algorithm presented in 
  Fuerst, S. V., & Wu, K. 2007, A&A, 474, 55, 
  and the radiative transfer formulation described in 
  Younsi, Z., Wu, K., & Fuerst, S. V. 2012, A&A, 545, A13

  Odyssey is distributed freely under the GNU general public license. 
  You can redistribute it and/or modify it under the terms of the License

  http://www.gnu.org/licenses/gpl.txt
  The current distribution website is:
  https://github.com/hungyipu/Odyssey/ 

 ***********************************************************************************/

#include <fstream>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <chrono>



// --- from constants.h ---
#define N					  6
#define PI					  3.14159265
#define C_G					  6.67259e-08
#define C_c					  2.99792458e+10
#define C_mSun					  1.99e+33
#define C_rgeo					  1.4774e+05        //C_G*C_msun/C_c/C_c
#define C_h					  6.6260755e-27	    //PlanCk Constant
#define C_kB					  1.380658e-16	    //Boltzmann constant
#define C_e					  4.8032068e-10
#define C_me					  9.1093897e-28
#define C_mp					  1.6726231e-24
#define C_Jansky				  1e-23
#define C_ly					  9.463e17
#define C_pc					  3.086e18
#define C_sgrA_mbh				  4.3e6             //mass of the black hole (Sgr A*)
#define C_sgrA_d				  8500              //distance to  the black hole (Sgr A*), in unit of pc
								  
#define IMAGE_SIZE				  1024
#define VarNUM					  9
#define r0					  Variables[0]
#define theta0  				  Variables[1]
#define a2   					  Variables[2]
#define Rhor    				  Variables[3]
#define Rmstable    			  	  Variables[4]
#define L   					  Variables[5]
#define kappa    				  Variables[6]
#define grid_x   				  Variables[7]
#define grid_y   				  Variables[8]
								  
// For Local Memory				  
#define VarINNUM				  4
#define A					  VariablesIn[0]
#define INCLINATION				  VariablesIn[1]
#define SIZE					  VariablesIn[2]
#define freq_obs				  VariablesIn[3]

// Mapping from threadIdx/blockIdx to pixel position
#define ResultsPixel(q)    	                  ResultsPixel[3 * (IMAGE_SIZE * (Y1) + (X1)) + q]
#define X1					  GridIdxX * GRID_DIM_X * BLOCK_DIM_X + BLOCK_DIM_X * _bid_x + _tid_x
#define Y1					  GridIdxY * GRID_DIM_Y * BLOCK_DIM_Y + BLOCK_DIM_Y * _bid_y + _tid_y
