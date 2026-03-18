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

// --- from hotspot.cu ---
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#ifdef RD_WG_SIZE_0_0                                                            
        #define BLOCK_SIZE RD_WG_SIZE_0_0                                        
#elif defined(RD_WG_SIZE_0)                                                      
        #define BLOCK_SIZE RD_WG_SIZE_0                                          
#elif defined(RD_WG_SIZE)                                                        
        #define BLOCK_SIZE RD_WG_SIZE                                            
#else                                                                                    
        #define BLOCK_SIZE 16                                                            
#endif                                                                                   

#define STR_SIZE 256

/* maximum power density possible (say 300W for a 10mm x 10mm chip)	*/
#define MAX_PD	(3.0e6)
/* required precision in degrees	*/
#define PRECISION	0.001
#define SPEC_HEAT_SI 1.75e6
#define K_SI 100
/* capacitance fitting factor	*/
#define FACTOR_CHIP	0.5

/* chip parameters	*/
float t_chip = 0.0005;
float chip_height = 0.016;
float chip_width = 0.016;
/* ambient temperature, assuming no package at all	*/
float amb_temp = 80.0;




#define IN_RANGE(x, min, max)   ((x)>=(min) && (x)<=(max))
#define CLAMP_RANGE(x, min, max) x = (x<(min)) ? min : ((x>(max)) ? max : x )
#define MIN(a, b) ((a)<=(b) ? (a) : (b))


/*
   compute N time steps
*/




