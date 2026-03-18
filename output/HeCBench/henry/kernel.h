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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>

#define NUMTHREADS 256  // number of threads per GPU block

// data for atom of crystal structure
//    Unit cell of crystal structure can then be stored 
//    as pointer array of StructureAtom's
struct StructureAtom {
  // Cartesian position, units: A
  double x;
  double y;
  double z;
  // Lennard-Jones epsilon parameter with adsorbate
  double epsilon;  // units: K
  // Lennard-Jones sigma parameter with adsorbate
  double sigma;  // units: A
};

// temperature, Kelvin
const double T = 298.0; 

// Universal gas constant, m3 - Pa / (K - mol)
const double R = 8.314; 

// Generate a random number 

// Compute the Boltzmann factor of methane at point (x, y, z) inside structure
//   Loop over all atoms of unit cell of crystal structure
//   Find nearest image to methane at point (x, y, z) for application of periodic boundary conditions
//   Compute energy contribution due to this atom via the Lennard-Jones potential

// Inserts a methane molecule at a random position inside the structure
// Calls function to compute Boltzmann factor at this point
// Stores Boltzmann factor computed at this thread in boltzmannFactors

