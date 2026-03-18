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
// https://www.particleincell.com/2016/cuda-pic/
// https://www.particleincell.com/wp-content/uploads/2016/02/sheath-gpu.cu

/* 1D sheath PIC simulation with HIP */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <chrono>
#include <hip/hip_runtime.h>

/*HIP error wraper*/

/*constants*/
#define EPS_0 8.85418782e-12 // F/m, vacuum permittivity
#define K 1.38065e-23        // J/K, Boltzmann constant
#define ME 9.10938215e-31    // kg, electron mass
#define QE 1.602176565e-19   // C, elementary charge
#define AMU 1.660538921e-27  // kg, atomic mass unit
#define EV_TO_K 11604.52     // 1eV in Kelvin, QE/K

/*simulation parameters, these could come from an input file*/
#define PLASMA_DEN 1e16      // plasma density to load
#define NUM_IONS 500000      // number of ions
#define NUM_ELECTRONS 500000 // number of electrons
#define DX 1e-4              // cell spacing
#define NC 100               // number of cells
#define NUM_TS 1000          // number of time steps
#define DT 1e-11             // time step size
#define ELECTRON_TEMP 3.0    // electron temperature in eV
#define ION_TEMP 1.0         // ion temperature in eV

/*domain parameters, set here so can access from GPU*/
#define X0 0           /*mesh origin*/
#define XL NC* DX      /*domain length*/
#define XMAX (X0 + XL) /*domain max position*/

const int THREADS_PER_BLOCK = 256;
/* Data structure to hold domain information*/
struct Domain
{
  const int ni      = NC + 1; /*number of nodes*/
  const double x0   = X0;
  const double dx   = DX;
  const double xl   = XL;
  const double xmax = XMAX;

  /*data structures*/
  double* phi; /*potential*/
  double* ef;  /*electric field on the cpu*/
  double* rho; /*charge density*/

  float* ndi; /*ion density on the CPU*/
  float* nde; /*electron density on the CPU*/
};

/* Data structure for particle storage **/
struct Particle
{
  double x;   /*position*/
  double v;   /*velocity*/
  bool alive; /*flag to avoid removing particles*/
};

/* Data structure to hold species information*/
struct Species
{
  double mass;   /*particle mass in kg*/
  double charge; /*particle charge in Coulomb*/
  double spwt;   /*species specific weight*/

  int np;             /*number of particles*/
  int np_alloc;       /*size of the allocated data array*/
  Particle* part;     /*array holding particles on the CPU*/
};

/** FUNCTION PROTOTYPES **/
double rnd();
double SampleVel(double v_th);
void ScatterSpecies(Species* species, Particle* species_part_gpu, float* den, float* den_gpu, double &time);
void ComputeRho(Species* ions, Species* electrons);
bool SolvePotential(double* phi, double* rho);
bool SolvePotentialDirect(double* phi, double* rho);
void ComputeEF(double* phi, double* ef, double* ef_gpu);
void PushSpecies(Species* species, Particle* species_part_gpu, double* ef);
void RewindSpecies(Species* species, Particle* species_part_gpu, double* ef);
void AddParticle(Species* species, double x, double v);
double XtoL(double pos);
double gather(double lc, const double* field);
void scatter(double lc, float value, float* field);

void WriteResults(int ts);

/* GLOBAL VARIABLES */
Domain domain;

FILE* file_res;

/* --------- main -------------*/

/***** HELPER FUNCTIONS *********************************************************/
/* random number generator
   for now using built-in but this is not adequate for real simulations*/

/* samples random velocity from Maxwellian distribution using Birdsall's method*/

/* move particles*/

/*scatter particles of species to the mesh*/

/*adds new particle to the species, returns pointer to the newly added data*/

/*computes charge density by adding ion and electron data*/

/*Thomas algorithm for a tri-diagonal matrix*/

/* solves potential using the Gauss Seidel Method, returns true if converged*/

/* computes electric field by differentiating potential*/

/*GPU code to move particles*/

/* moves particles of a single species, returns wall charge*/

/*GPU code to rewind particles*/

/* rewinds particle velocities by -0.5DT*/

/* converts physical coordinate to logical*/

/* atomic scatter of scalar value onto a field at logical coordinate lc*/

/* gathers field value at logical coordinate lc*/

/* writes new zone to the results file*/
