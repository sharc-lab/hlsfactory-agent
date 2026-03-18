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

// --- from init.cu ---
/****************************************************************************
 *
 * init.cu, Version 1.0.0 Mon 09 Jan 2012
 *
 * ----------------------------------------------------------------------------
 *
 * CUDA EGS
 * Copyright (C) 2012 CancerCare Manitoba
 *
 * The latest version of CUDA EGS and additional information are available online at 
 * http://www.physics.umanitoba.ca/~elbakri/cuda_egs/ and http://www.lippuner.ca/cuda_egs
 *
 * CUDA EGS is free software; you can redistribute it and/or modify it under the 
 * terms of the GNU General Public License as published by the Free Software 
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.                                       
 *                                                                           
 * CUDA EGS is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
 * details.                              
 *                                                                           
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * ----------------------------------------------------------------------------
 *
 *   Contact:
 *
 *   Jonas Lippuner
 *   Email: jonas@lippuner.ca 
 *
 ****************************************************************************/

#ifdef CUDA_EGS

// remove all control characters (possibly including spaces) from a string

// read the Mersenne Twister (MT) parameters from the parameter file, initialize the MTs 
// with the given seed and copy the relevant data to the device

// allocate memory for the stack and associated counters

// read the source parameters from the input file and copy the relevant data to the device

// read the detector parameters from the input file and copy the relevant data to the device

// populate the region data with the values read from the egsphant file and copy it to the device

// read the egsphant file and copy the phantom data to the device

// call the above functions to perfrom the initialization

// free all allocated memory on the host and the device

#endif


// --- from kernels.cu ---
/****************************************************************************
 *
 * kernels.cu, Version 1.0.0 Mon 09 Jan 2012
 *
 * ----------------------------------------------------------------------------
 *
 * Copyright (C) 2012 CancerCare Manitoba
 *
 * The latest version of CUDA EGS and additional information are available online at 
 * http://www.physics.umanitoba.ca/~elbakri/cuda_egs/ and http://www.lippuner.ca/cuda_egs
 *
 * CUDA EGS is free software; you can redistribute it and/or modify it under the 
 * terms of the GNU General Public License as published by the Free Software 
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.                                       
 *                                                                           
 * CUDA EGS is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
 * details.                              
 *                                                                           
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * ----------------------------------------------------------------------------
 *
 *   Contact:
 *
 *   Jonas Lippuner
 *   Email: jonas@lippuner.ca 
 *
 ****************************************************************************/

#ifdef CUDA_EGS
#define MASK 0xFFFFFFFF

/* * * * * * * * * * * * * * *
 * General Helper Functions  *
 * * * * * * * * * * * * * * */

// the maximum number of ULPs that two floats may differ and still be considered "almost equal"
#define MAX_ULPS 20

// check whether two floats are almost equal
// taken from http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm

// calculate indices of this thread

/* * * * * * * * * * * * * * * * * * *
 * Random Number Generator Functions *
 * * * * * * * * * * * * * * * * * * */

/****************************************************************************
 * ALL THESE FUNCTIONS MUST *ALWAYS* BE CALLED BY *ALL* THREADS IN THE WARP *
 ****************************************************************************/

// update the array with random numbers for one warp
// This implements the Mersenne Twister for Graphic Processors (MTGP) and the
// code is largely based on the code available at http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/MTGP/index.html

// read the status of the MT for this warp from global memory

// write the status of the MT for this warp to global memory

// get the next random number

/* * * * * * * * * * * *
 * Geometry Functions  *
 * * * * * * * * * * * */

// Determine the index i such that the point p lies between bounds[i] and bounds[i+1].
// Code was taken from the function isWhere of the class EGS_PlanesT in the file
// egs_planes.h (v 1.17 2009/07/06) and the function findRegion of the class EGS_BaseGeometry
// in the file egs_base_geometry.h (v 1.26 2008/09/22) of the EGSnrc C++ Class Library.

// Determine the distance t to the next voxel boundary for the particle p and return
// the region index that the particle will enter.
// Code was taken from the function howfar of the class EGS_XYZGeometry in the file
// egs_nd_geometry.h (v 1.26 2009/07/06) of the EGSnrc C++ Class Library.
// Our region indices are shifted by 1 because our outside region is 0, while
// the outside region in the EGSnrc C++ Class Library is -1.

/**********************************************************************
 * THIS FUNCTION MUST *ALWAYS* BE CALLED BY *ALL* THREADS IN THE WARP *
 **********************************************************************/
// This is the subroutine UPHI(IENTRY,LVL) with IENTRY = 2 and LVL = 1 in the file 
// egsnrc.mortran (v 1.72 2011/05/05) of the EGSnrc code system.
// However, note that we are not using the box method implemented in the macro 
// $SELECT-AZIMUTHAL-ANGLE, because that involves a sampling loop and then all
// threads would have to wait until the last thread has finished the loop. Calculating
// sin and cos with __sincosf is not that expensive, so we use that instead.
// Note that this is based on assumption and was not experimentally verified.

// Add the weight wt and energy e to the pixel (x,y) in the category cat. atomicAdd is
// used to avoid data hazards if multiple threads try to update the same pixel at the 
// same time.

/* * * * * * * * * * * * * * *
 * Simulation Step Functions *
 * * * * * * * * * * * * * * */

// Create a new particle.
// This is essentially the function getNextParticle of the class EGS_CollimatedSource in the
// EGSnrc C++ Class Library with a source shape EGS_PointShape, target shape EGS_RectangleShape
// and a spectrum EGS_MonoEnergy or EGS_TabulatedSpectrum (the underlying EGS_AliasTable has 
// type = 1, i.e. it is a histogram).

// Photon energy fell below the cutoff energy, destroy the photon.

// Propagate the photon to the detector.

// Transport the photon one step through the phantom and determine which (if any) interaction
// takes place next.
// This is the subroutine PHOTON in the file egsnrc.mortran (v 1.72 2011/05/05) of the EGSnrc 
// code system.

// Perform a Rayleigh interaction.
// This is the subroutine egs_rayleigh_sampling in the file egsnrc.mortran (v 1.72 2011/05/05) 
// of the EGSnrc code system.

// Perform a Compton interaction.
// This is a simplified version of the subroutine COMPT in the file egsnrc.mortran (v 1.72 2011/05/05) 
// of the EGSnrc code system. The simplification is that we do not consider bound compton scattering,
// that we always use Klein-Nishina and do not create an electron.

// Perform a pair production interaction. Since we do not consider electrons, we just destroy the photon.

// Perform a photo electric interaction. Since we do not consider electrons, we just destroy the photon.

/* * * * * *
 * Kernels *
 * * * * * */

// this is the simulation kernel
/* * * * * * * * * * * * * * * * * * * * * * * * * *
 * Number of blocks: SIMULATION_NUM_BLOCKS         *
 * Number of warps:  SIMULATION_WARPS_PER_BLOCK    *
 * * * * * * * * * * * * * * * * * * * * * * * * * */

// this is the summing kernel
/* * * * * * * * * * * * * * * * * * * * * * * * * *
 * Number of blocks: SUM_DETECTOR_NUM_BLOCKS       *
 * Number of warps:  SUM_DETECTOR_WARPS_PER_BLOCK  *
 * * * * * * * * * * * * * * * * * * * * * * * * * */

#endif


// --- from main.cu ---
/****************************************************************************
 *
 * main.cu, Version 1.0.0 Mon 09 Jan 2012
 *
 * ----------------------------------------------------------------------------
 *
 * CUDA EGS
 * Copyright (C) 2012 CancerCare Manitoba
 *
 * The latest version of CUDA EGS and additional information are available online at 
 * http://www.physics.umanitoba.ca/~elbakri/cuda_egs/ and http://www.lippuner.ca/cuda_egs
 *
 * CUDA EGS is free software; you can redistribute it and/or modify it under the 
 * terms of the GNU General Public License as published by the Free Software 
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.                                       
 *                                                                           
 * CUDA EGS is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
 * details.                              
 *                                                                           
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * ----------------------------------------------------------------------------
 *
 *   Contact:
 *
 *   Jonas Lippuner
 *   Email: jonas@lippuner.ca 
 *
 ****************************************************************************/

#define CUDA_EGS

#include "output.c"
#include "media.c"





// --- from EGS.h ---
/****************************************************************************
 *
 * CUDA_EGS.h, Version 1.0.0 Mon 09 Jan 2012
 *
 * ----------------------------------------------------------------------------
 *
 * CUDA EGS
 * Copyright (C) 2012 CancerCare Manitoba
 *
 * The latest version of CUDA EGS and additional information are available online at 
 * http://www.physics.umanitoba.ca/~elbakri/cuda_egs/ and http://www.lippuner.ca/cuda_egs
 *
 * CUDA EGS is free software; you can redistribute it and/or modify it under the 
 * terms of the GNU General Public License as published by the Free Software 
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.                                       
 *                                                                           
 * CUDA EGS is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
 * details.                              
 *                                                                           
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * ----------------------------------------------------------------------------
 *
 *   Contact:
 *
 *   Jonas Lippuner
 *   Email: jonas@lippuner.ca 
 *
 ****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <cstdarg>
#include <string>
#include <time.h>
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;

/*****************
 * CONFIGURATION *
 *****************/

// If you want to use an energy spectrum for the source, uncomment the following line,
// if you want to use a monoenergetic source, comment the following line.

#define USE_ENERGY_SPECTRUM

// If you want to measure the average number of iterations of the inner loop perfored in one
// iteration of the outer loop, uncomment the following line. This number provides a good
// indication of the average thread idleness, but will probably slighly decrease the overall
// performance of the simulation.

#define DO_LIST_DEPTH_COUNT

// warp size
#define WARP_SIZE 32

// number of multiprocessors
#define NUM_MULTIPROC 80

// The following is the number of warps in each block. This should be large enough to ensure
// a good occupancy, but it is limited by the available registers and shared memory.
#define SIMULATION_WARPS_PER_BLOCK 16

// The following is the number of blocks that are launched for each multiprocessor. There is
// probably no reason for this to be much larger than 1. More blocks require more global memory.
#define SIMULATION_BLOCKS_PER_MULTIPROC 1

// The following is the number of iterations of the outer loop per simulation kernel. A larger
// number will increase the performance of the simulation because fewer kernels launches will
// be necessary, which all have an overhead cost. However, a larger number will also increase 
// the accumulative effect of single precision rounding errors, thus potentially decreasing
// the accuracy of the simulation.
#define SIMULATION_ITERATIONS 32768

/*************
 * CONSTANTS *
 *************/

#define PI                          3.1415926535F
#define ELECTRON_REST_MASS_FLOAT    0.5110034F          // MeV * c^(-2)
#define ELECTRON_REST_MASS_DOUBLE   0.5110034           // MeV * c^(-2)
#define HC_INVERSE                  80.65506856998F     // (hc)^(-1) in (Angstrom * MeV)^(-1)
#define TWICE_HC2                   0.000307444456F     // 2*(hc)^2 in (Angstrom * Mev)^2

// category names
// p primary (never scattered)
// c compton (Compton scattered once)
// r rayleigh (Rayleigh scattered once)
// m multiple (scattered more than once)
// t total (all photons)
const char categories[] = "pcrmt";

// buffer to read or write strings
#define CHARLEN	1024
char charBuffer[CHARLEN];

const char *input_file;
const char *egsphant_file = "./data/EGS_phantom_32.egsphant";
const char *pegs_file = "./data/EGS_phantom.pegs4dat";
const char *MT_params_file = "./data/MTGP_3217_0-8191.bin";
const char *photon_xsections = "./data/si"; 
const char *atomic_ff_file = "./data/pgs4form.dat";
const char *spec_file = "./data/tungsten-80kVp-4mmAl.spectrum";

/**********************************
 * MISCELLANEOUS TYPE DEFINITIONS *
 **********************************/

typedef unsigned char           uchar;

// the different indices of a thread
typedef struct indices {
    uint b;     // index of the block in the grid
    uchar w;    // index of the warp in the block
    uchar t;    // index of the thread in the warp
    uint p;     // index of the particle on the stack
} indices;

/**************************
 * SIMULATION DEFINITIONS *
 **************************/

// the number of blocks used to run the simulation kernel
#define SIMULATION_NUM_BLOCKS (SIMULATION_BLOCKS_PER_MULTIPROC * NUM_MULTIPROC)

// all data of one particle
typedef struct particle_t {
    uchar   status;     // the current (or next) simulation step for this particle
    uchar   reserved;   // currently not used
    char    charge;     // charge of the particle (always 0, since only photons are considered)
    bool    process;    // bool indicating whether the particle needs to perform the current 
                        // simulation step
    float   e;          // energy
    float   wt;         // statistical weight
    uint    region;     // current region
    uint    latch;      // variable for tracking scatter events
    
    // position
    float   x;
    float   y;
    float   z;

    // direction
    float   u;
    float   v;
    float   w;
} particle_t;

// we split up the data for each particle into 16-byte (128-bit) blocks (one uint4) so that we get
// coalesced global memory accesses
typedef struct stack_t {
    
    // 1st block
    uint4   *a;
    /* consists of
    uchar   status;     // 1 byte
    uchar   reserved;   // 1 byte
    char    charge;     // 1 byte
    bool    process;    // 1 byte
    float   e;          // 4 bytes
    float   wt;         // 4 bytes
    uint    region;     // 4 bytes
    */

    // 2nd block
    uint4   *b;
    /* consists of
    uint    latch;      // 4 bytes
    float   x;          // 4 bytes
    float   y;          // 4 bytes
    float   z;          // 4 bytes
    */

    // 3rd block
    uint4   *c;
    /* consists of
    float   u;          // 4 bytes
    float   v;          // 4 bytes
    float   w;          // 4 bytes
    [not used]          // 4 bytes
    */
} stack_t;

stack_t     d_stack;
stack_t    stack;

enum particle_status {
    p_cutoff_discard    = 0x00,
    p_user_discard      = 0x01,
    p_photon_step       = 0x02,
    p_rayleigh          = 0x03,
    p_compton           = 0x04,
    p_photo             = 0x05,
    p_pair              = 0x06,
    p_new_particle      = 0x07,
    p_empty             = 0x08 
};

// number of different particle statuses
#define NUM_CAT 9
// number of different detector categories (primary, compton, rayleigh, multiple)
#define NUM_DETECTOR_CAT 4

// number of blocks of the summing kernel
#define SUM_DETECTOR_NUM_BLOCKS (2 * NUM_DETECTOR_CAT)
// warps per block of the summing kernel
#define SUM_DETECTOR_WARPS_PER_BLOCK 32

// list depth counter
#ifdef DO_LIST_DEPTH_COUNT
uint list_depth_shared[SIMULATION_WARPS_PER_BLOCK];
uint num_inner_iterations_shared[SIMULATION_WARPS_PER_BLOCK];
typedef ulong total_list_depth_t[SIMULATION_NUM_BLOCKS];
typedef ulong total_num_inner_iterations_t[SIMULATION_NUM_BLOCKS];
total_list_depth_t *d_total_list_depth, *h_total_list_depth;
total_num_inner_iterations_t *d_total_num_inner_iterations, *h_total_num_inner_iterations;
total_list_depth_t *total_list_depth;
total_num_inner_iterations_t *total_num_inner_iterations;
#endif

uint step_counters_shared[SIMULATION_WARPS_PER_BLOCK][NUM_CAT];
double combined_weight_list_shared[SIMULATION_WARPS_PER_BLOCK];
float weight_list_shared[SIMULATION_WARPS_PER_BLOCK][WARP_SIZE];

typedef float *detector_scores_t[SIMULATION_NUM_BLOCKS][NUM_DETECTOR_CAT];
typedef double total_weights_t[SIMULATION_NUM_BLOCKS];
typedef ulong total_step_counts_t[SIMULATION_NUM_BLOCKS][NUM_CAT];

detector_scores_t d_detector_scores_count, d_detector_scores_energy;
double *d_detector_totals_count[NUM_DETECTOR_CAT], *d_detector_totals_energy[NUM_DETECTOR_CAT];
total_weights_t *d_total_weights;
total_step_counts_t *d_total_step_counts, *h_total_step_counts;

detector_scores_t detector_scores_count, detector_scores_energy;
double *detector_totals_count[NUM_DETECTOR_CAT], *detector_totals_energy[NUM_DETECTOR_CAT];
total_weights_t *total_weights;
total_step_counts_t *total_step_counts;

/********************************
 * MERSENNE TWISTER DEFINITIONS *
 ********************************/

#define MT_EXP 3217
// number of elements in the status array
#define MT_N (MT_EXP / 32 + 1)
// next larger multiple of WARP_SIZE
#define MT_NUM_STATUS (((MT_N - 1) / WARP_SIZE + 1) * WARP_SIZE)
// number of random numbers that each thread can use until a status update is necessary
#define MT_NUM_PER_THREAD (MT_N / WARP_SIZE)
#define MT_TABLE_SIZE 16

typedef struct MT_input_param {
    uint    mexp;
    uint    bit_size;
    uint    id;
    uint    M;              // also called pos in the MTGP code
    uint    sh1;
    uint    sh2;
    uint    tbl[4];
    uint    tmp_tbl[4];
    uint    mask;
} MT_input_param;

typedef struct MT_param {
    uint    M;
    uint    sh1;
    uint    sh2;
    uint    mask;
} MT_param;

typedef struct MT_tables_t {
    uint    recursion[MT_TABLE_SIZE];
    uint    tempering[MT_TABLE_SIZE];
} MT_tables_t;

MT_param    MT_params_shared[SIMULATION_WARPS_PER_BLOCK];
uint        MT_statuses_shared[SIMULATION_WARPS_PER_BLOCK][MT_N];
MT_tables_t MT_tables_shared[SIMULATION_WARPS_PER_BLOCK];
uchar       rand_idx_shared[SIMULATION_WARPS_PER_BLOCK];
float       random_array_shared[SIMULATION_WARPS_PER_BLOCK][WARP_SIZE * MT_NUM_PER_THREAD];

MT_param    *h_MT_params, *d_MT_params;
uint		*h_MT_statuses, *d_MT_statuses;
MT_tables_t *d_MT_tables;

uint *MT_statuses;
MT_param *MT_params;
MT_tables_t *MT_tables;

/************************
 * GEOMETRY DEFINITIONS *
 ************************/

// source
typedef struct source_t {
#ifdef USE_ENERGY_SPECTRUM
    uint    n;
    float   *xi, *wi;
    int     *bin;
#else
	float   energy;
#endif
    float3  source_point;
    float   rectangle_z;
    float2  rectangle_min;
    float2  rectangle_max;
    float2  rectangle_size;
    float   rectangle_area;
} source_t; 

typedef struct detector_t {
    float3  center;
    float2  d;
    uint2   N;
} detector_t;

typedef struct phantom_t {
    uint3   N;
    float   *x_bounds;
    float   *y_bounds;
    float   *z_bounds;
} phantom_t;

detector_t  h_detector;
source_t    h_source;
phantom_t   h_phantom;

detector_t  detector;
source_t    source;
phantom_t   phantom;

/*******************************
 * SIMULATION DATA DEFINITIONS *
 *******************************/

#define BOUND_COMPTON_MASK 0x000EU
#define VACUUM 0xFFFFU
#define VACUUM_STEP 1E8F
#define EPSGMFP 1E-5F
#define SMALL_POLAR_ANGLE_THRESHOLD 1E-20F

enum region_flags {
    f_rayleigh              = 0x0001U,                  // 0000 0000 0000 0001
    f_bound_compton         = 0x0002U,                  // 0000 0000 0000 0010
    f_bound_compton_2       = 0x0006U,                  // 0000 0000 0000 0110
    f_bound_compton_3       = 0x000AU,                  // 0000 0000 0000 1010
    f_bound_compton_4       = 0x000EU,                  // 0000 0000 0000 1110
    f_atomic_relaxation     = 0x0010U,                  // 0000 0000 0001 0000
    f_photo_electron_angular_distribution = 0x0020U,    // 0000 0000 0010 0000
    f_range_rejection       = 0x0040U                   // 0000 0000 0100 0000
};

typedef struct __align__(16) region_data_t {
    ushort  med;
    ushort  flags;
    float   rhof;
    float   pcut;
    float   ecut;
} region_data_t;

region_data_t *d_region_data;
region_data_t *region_data;

/**************************
 * MEDIA DATA DEFINITIONS *
 **************************/

const char *data_dir;

