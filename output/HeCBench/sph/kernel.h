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

// --- from fileio.cu ---
////////////////////////////////////////////////
// File input/output functions
////////////////////////////////////////////////

#include <stdio.h>

static int fileNum = 0;

// Write fluid particle data to file

// Write boundary particle data to file


// --- from fluid.cu ---
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>

////////////////////////////////////////////////////////////////////////////
// B spline smoothing kernel
////////////////////////////////////////////////////////////////////////////


// Gradient of B spline kernel

////////////////////////////////////////////////////////////////////////////
// Boundary particle force
// http://iopscience.iop.org/0034-4885/68/8/R01/pdf/0034-4885_68_8_R01.pdf
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// Particle attribute computations
////////////////////////////////////////////////////////////////////////////






// Update particle acclerations


// Update particle positions
// Leap Frog integration with v(t+1) estimated

// Seed simulation with Euler step v(t-dt/2) needed by leap frog integrator
// Should calculate all accelerations but assuming just g simplifies acc port

// Initialize particles





// --- from geometry.cu ---
///////////////////////////////////////////////////////////////
//  Functions related to problem geometry
///////////////////////////////////////////////////////////////

#include <stdio.h>
#include <math.h>

///////////////////////////////////////////////////////////////
// Construct the particle boundary box
// Setting particle normals require the explicit construction
///////////////////////////////////////////////////////////////


// --- from common.h ---
#ifndef common_
#define common_

////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////

struct boundary_particle {
    double3 pos; // position
    double3 n;   // position
} ;

struct fluid_particle {
    double density;
    double pressure;
    double3 pos;     // position
    double3 v;       // velocity
    double3 v_half;  // half step velocity
    double3 a;       // acceleration
};

struct param {
    double rest_density;
    double mass_particle;
    double spacing_particle;
    double smoothing_radius;
    double g;
    double time_step;
    double alpha;
    double surface_tension;
    double speed_sound;
    int number_particles;
    int number_fluid_particles;
    int number_boundary_particles;
    int number_steps;
    int steps_per_frame;
}; // Simulation paramaters

struct AABB {
    double min_x;
    double max_x;
    double min_y;
    double max_y;
    double min_z;
    double max_z;
} ; //Axis aligned bounding box

////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////

void constructBoundaryBox(boundary_particle *boundary_particles, AABB* boundary, param *params);
void eulerStart(fluid_particle* fluid_particles, boundary_particle *boundary_particles, param *params);
void initParticles(fluid_particle** fluid_particles, boundary_particle** boundary_particles, AABB* water, AABB* boundary, param* params);
void initParams(AABB* water_volume, AABB* boundary_volume, param* params);
void finalizeParticles(fluid_particle *fluid_particles, boundary_particle *boundary_particles);
void writeFile(fluid_particle *particles, param *params);
void writeBoundaryFile(boundary_particle *boundary, param *params);

#endif

