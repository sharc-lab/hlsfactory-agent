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

// --- from euler3d.cu ---
// Copyright 2009, Andrew Corrigan, acorriga@gmu.edu
// This code is from the AIAA-2009-4001 paper

//#include <cutil.h>
#include <iostream>
#include <fstream>

 
 
/*
 * Options 
 * 
 */ 
#define GAMMA 1.4f
#define iterations 2000
// #ifndef block_length
// 	#define block_length 192
// #endif

#define NDIM 3
#define NNB 4

#define RK 3	// 3rd order RK
#define ff_mach 1.2f
#define deg_angle_of_attack 0.0f

/*
 * not options
 */

#ifdef RD_WG_SIZE_0_0
	#define BLOCK_SIZE_0 RD_WG_SIZE_0_0
#elif defined(RD_WG_SIZE_0)
	#define BLOCK_SIZE_0 RD_WG_SIZE_0
#elif defined(RD_WG_SIZE)
	#define BLOCK_SIZE_0 RD_WG_SIZE
#else
	#define BLOCK_SIZE_0 192
#endif

#ifdef RD_WG_SIZE_1_0
	#define BLOCK_SIZE_1 RD_WG_SIZE_1_0
#elif defined(RD_WG_SIZE_1)
	#define BLOCK_SIZE_1 RD_WG_SIZE_1
#elif defined(RD_WG_SIZE)
	#define BLOCK_SIZE_1 RD_WG_SIZE
#else
	#define BLOCK_SIZE_1 192
#endif

#ifdef RD_WG_SIZE_2_0
	#define BLOCK_SIZE_2 RD_WG_SIZE_2_0
#elif defined(RD_WG_SIZE_1)
	#define BLOCK_SIZE_2 RD_WG_SIZE_2
#elif defined(RD_WG_SIZE)
	#define BLOCK_SIZE_2 RD_WG_SIZE
#else
	#define BLOCK_SIZE_2 192
#endif

#ifdef RD_WG_SIZE_3_0
	#define BLOCK_SIZE_3 RD_WG_SIZE_3_0
#elif defined(RD_WG_SIZE_3)
	#define BLOCK_SIZE_3 RD_WG_SIZE_3
#elif defined(RD_WG_SIZE)
	#define BLOCK_SIZE_3 RD_WG_SIZE
#else
	#define BLOCK_SIZE_3 192
#endif

#ifdef RD_WG_SIZE_4_0
	#define BLOCK_SIZE_4 RD_WG_SIZE_4_0
#elif defined(RD_WG_SIZE_4)
	#define BLOCK_SIZE_4 RD_WG_SIZE_4
#elif defined(RD_WG_SIZE)
	#define BLOCK_SIZE_4 RD_WG_SIZE
#else
	#define BLOCK_SIZE_4 192
#endif

// #if block_length > 128
// #warning "the kernels may fail too launch on some systems if the block length is too large"
// #endif

#define VAR_DENSITY 0
#define VAR_MOMENTUM  1
#define VAR_DENSITY_ENERGY (VAR_MOMENTUM+NDIM)
#define NVAR (VAR_DENSITY_ENERGY+1)

/*
 * Generic functions
 */
template <typename T>

template <typename T>

template <typename T>

template <typename T>

template <typename T>


/*
 * Element-based Cell-centered FVM solver functions
 */
float ff_variable[NVAR];
float3 ff_flux_contribution_momentum_x[1];
float3 ff_flux_contribution_momentum_y[1];
float3 ff_flux_contribution_momentum_z[1];
float3 ff_flux_contribution_density_energy[1];



	




/*
 *
 *
*/


/*
 * Main function
 */


// --- from euler3d_double.cu ---
// Copyright 2009, Andrew Corrigan, acorriga@gmu.edu
// This code is from the AIAA-2009-4001 paper

// #include <cutil.h>
#include <iostream>
#include <fstream>

#if CUDART_VERSION < 3000
struct double3
{
	double x, y, z;
};
#endif

/*
 * Options 
 * 
 */ 
#define GAMMA 1.4
#define iterations 2000
#ifndef block_length
	#define block_length 128
#endif

#define NDIM 3
#define NNB 4

#define RK 3	// 3rd order RK
#define ff_mach 1.2
#define deg_angle_of_attack 0.0

/*
 * not options
 */

#if block_length > 128
#warning "the kernels may fail too launch on some systems if the block length is too large"
#endif

#define VAR_DENSITY 0
#define VAR_MOMENTUM  1
#define VAR_DENSITY_ENERGY (VAR_MOMENTUM+NDIM)
#define NVAR (VAR_DENSITY_ENERGY+1)

/*
 * Generic functions
 */
template <typename T>

template <typename T>

template <typename T>

template <typename T>

template <typename T>


/*
 * Element-based Cell-centered FVM solver functions
 */
double ff_variable[NVAR];
double3 ff_flux_contribution_momentum_x[1];
double3 ff_flux_contribution_momentum_y[1];
double3 ff_flux_contribution_momentum_z[1];
double3 ff_flux_contribution_density_energy[1];



	




/*
 *
 *
*/


/*
 * Main function
 */


// --- from pre_euler3d.cu ---
// Copyright 2009, Andrew Corrigan, acorriga@gmu.edu
// This code is from the AIAA-2009-4001 paper

// #include <cutil.h>

#include <iostream>
#include <fstream>

 
 
/*
 * Options 
 * 
 */ 
#define GAMMA 1.4
#define iterations 2000
#ifndef block_length
	#define block_length 192
#endif

#define NDIM 3
#define NNB 4

#define RK 3	// 3rd order RK
#define ff_mach 1.2
#define deg_angle_of_attack 0.0f

/*
 * not options
 */

#if block_length > 128
#warning "the kernels may fail too launch on some systems if the block length is too large"
#endif

#define VAR_DENSITY 0
#define VAR_MOMENTUM  1
#define VAR_DENSITY_ENERGY (VAR_MOMENTUM+NDIM)
#define NVAR (VAR_DENSITY_ENERGY+1)

/*
 * Generic functions
 */
template <typename T>

template <typename T>

template <typename T>

template <typename T>

template <typename T>


/*
 * Element-based Cell-centered FVM solver functions
 */
float ff_variable[NVAR];
float3 ff_fc_momentum_x[1];
float3 ff_fc_momentum_y[1];
float3 ff_fc_momentum_z[1];
float3 ff_fc_density_energy[1];



	







/*
 * Main function
 */


// --- from pre_euler3d_double.cu ---
// Copyright 2009, Andrew Corrigan, acorriga@gmu.edu
// This code is from the AIAA-2009-4001 paper

#include <iostream>
#include <fstream>

#if CUDART_VERSION < 3000
struct double3
{
	double x, y, z;
};
#endif

/*
 * Options 
 * 
 */ 
#define GAMMA 1.4
#define iterations 2000
#ifndef block_length
	#define block_length 128
#endif

#define NDIM 3
#define NNB 4

#define RK 3	// 3rd order RK
#define ff_mach 1.2
#define deg_angle_of_attack 0.0

/*
 * not options
 */

#if block_length > 128
#warning "the kernels may fail too launch on some systems if the block length is too large"
#endif

#define VAR_DENSITY 0
#define VAR_MOMENTUM  1
#define VAR_DENSITY_ENERGY (VAR_MOMENTUM+NDIM)
#define NVAR (VAR_DENSITY_ENERGY+1)

/*
 * Generic functions
 */
template <typename T>

template <typename T>

template <typename T>

template <typename T>

template <typename T>


/*
 * Element-based Cell-centered FVM solver functions
 */
double ff_variable[NVAR];
double3 ff_fc_momentum_x[1];
double3 ff_fc_momentum_y[1];
double3 ff_fc_momentum_z[1];
double3 ff_fc_density_energy[1];



	





/*
 *
 *
*/


/*
 * Main function
 */
