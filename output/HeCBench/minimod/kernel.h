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

// --- from constants.cu ---

// Constants
const float _fmax = 25.0f;
const float vmin = 1500.f;
const float vmax = 4500.f;
const float cfl = 0.8f;


// --- from data_setup.cu ---
#include <float.h>
#include <math.h>






// --- from grid.cu ---
#include <stdio.h>


// Lead padding needed to align element (0,0,ndampz) on 128B cache line

// Useful size of the grid, in bytes

// Device grid, with lead padding
  int leadpad = getLeadpad(grid);

}

// Host grid, with lead padding



// --- from main.cu ---
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>








// --- from minimig.cu ---
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <time.h>

#define R 4
#define NDIM 8






// --- from pml.cu ---
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @param profile has dimension [i_min,i_max]
 */





// --- from constants.h ---
#ifndef CONSTANTS_H
#define CONSTANTS_H

#define POW2(x) ((x)*(x))
#define IDX3(i,j,k)((llint)((i + lx) * ldimy + j + ly) * (llint)ldimz + k + lz)
#define IDX3_grid(i, j, k, grid) (((i + grid.lx) * grid.ldimy + j + grid.ly) * grid.ldimz + k + grid.lz)

extern const float _fmax;
extern const float vmin;
extern const float vmax;
extern const float cfl;

typedef long long int llint;
typedef unsigned int uint;

#endif


// --- from data_setup.h ---
#ifndef DATA_SETUP_H
#define DATA_SETUP_H


void target_init(struct grid_t grid, uint nsteps,
                 const float * u, const float * v, const float * phi,
                 const float * eta, const float * coefx, const float * coefy,
                 const float * coefz, const float * vp, const float * source);

void target(uint nsteps, double *time_kernel,
            struct grid_t grid,
            llint sx, llint sy, llint sz,
            float hdx_2, float hdy_2, float hdz_2,
            const float * coefx, const float * coefy, const float * coefz,
            float * u, const float * v, const float * vp,
            const float * phi, const float * eta, const float * source);

void target_finalize(struct grid_t grid, uint nsteps,
                     const float * u, const float * v, const float * phi,
                     const float * eta, const float * coefx, const float * coefy,
                     const float * coefz, const float * vp, const float * source);

void kernel_add_source(struct grid_t grid,
                       float * u, const float * source, llint istep,
                       llint sx, llint sy, llint sz);

void find_min_max_u(struct grid_t grid,
                    const float * u, float * min_u, float * max_u);

#endif


// --- from grid.h ---
#ifndef GRID_H
#define GRID_H


struct grid_t {
    llint ntaperx, ntapery, ntaperz;
    llint ndampx, ndampy, ndampz;
    llint nx, ny, nz;
    llint ldimx, ldimy, ldimz;
    llint dx, dy, dz;
    llint x1, x2, x3, x4, x5, x6;
    llint y1, y2, y3, y4, y5, y6;
    llint z1, z2, z3, z4, z5, z6;
    llint lx, ly, lz;
    // These parameters are used only for tasks
    llint ntx, nty, tsx, tsy;
};

struct grid_t init_grid(llint nx, llint ny, llint nz, llint tsx, llint tsy);

// Allocate or release a grid on GPU
float * allocateDeviceGrid (struct grid_t grid);
void freeDeviceGrid (float *ptr, struct grid_t grid);

// Allocate or release a grid on host
float * allocateHostGrid (struct grid_t grid);
void freeHostGrid (float *ptr, struct grid_t grid);

// Useful size of the grid, in bytes
size_t gridSize (struct grid_t grid);

#endif


// --- from pml.h ---
#ifndef PML_H
#define PML_H


void init_eta(struct grid_t grid, float dt_sch, float *eta);

#endif
