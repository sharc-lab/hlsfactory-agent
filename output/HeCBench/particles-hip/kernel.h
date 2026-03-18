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

// --- from bitonicSort.cu ---
#include "hip/hip_runtime.h"
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *



// --- from bitonicSort_kernels.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *


////////////////////////////////////////////////////////////////////////////////
// Monolithic bitonic sort kernel for short arrays fitting into local memory
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Bitonic sort kernel for large arrays (not fitting into local memory)
////////////////////////////////////////////////////////////////////////////////
//Bottom-level bitonic sort
//Almost the same as bitonicSortLocal with the only exception
//of even / odd subarrays (of LOCAL_SIZE_LIMIT points) being
//sorted in opposite directions

//Bitonic merge iteration for 'stride' >= LOCAL_SIZE_LIMIT

//Combined bitonic merge steps for
//'size' > LOCAL_SIZE_LIMIT and 'stride' = [1 .. LOCAL_SIZE_LIMIT / 2]


// --- from main.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <hip/hip_runtime.h>
#include "particles.h"

#define MAX_EPSILON_ERROR 5.00f
#define THRESHOLD         0.30f
#define GRID_SIZE         64
#define NUM_PARTICLES     16384

// Simulation parameters
const float timestep = 0.5f;              // Time slice for re-computation iteration
//const float gravity = 0.0005f;            // Strength of gravity
//const float damping = 1.0f;
const float fParticleRadius = 0.023f;     // Radius of individual particles
const float fColliderRadius = 0.17f;      // Radius of collider for interacting with particles in 'm' mode
//const float collideSpring = 0.4f;         // Elastic spring constant for impact between particles
//const float collideDamping = 0.025f;      // Inelastic loss component for impact between particles
//const float collideShear = 0.12f;         // Friction constant for particles in contact
//const float collideAttraction = 0.0012f;  // Attraction between particles (~static or Van der Waals) 



// Main program


// --- from particles.cu ---
#include "hip/hip_runtime.h"
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

//Standard utilities and systems includes
#include <stdio.h>
#include <hip/hip_runtime.h>
#include "particles.h"

//Simulation parameters

static const size_t wgSize = 64;








// --- from particles_kernels.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *

////////////////////////////////////////////////////////////////////////////////
// Save particle grid cell hashes and indices
////////////////////////////////////////////////////////////////////////////////

//Calculate address in grid from position (clamping to edges)

//Calculate grid hash value for each particle

////////////////////////////////////////////////////////////////////////////////
// Find cell bounds and reorder positions+velocities by sorted indices
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Process collisions (calculate accelerations)
////////////////////////////////////////////////////////////////////////////////

