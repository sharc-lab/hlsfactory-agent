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

// --- from GridInit.cu ---



// --- from Main.cu ---

#ifdef MPI
#include<mpi.h>
#endif



// --- from Materials.cu ---
// Material data is hard coded into the functions in this file.
// Note that there are 12 materials present in H-M (large or small)


// num_nucs represents the number of nuclides that each material contains

// Assigns an array of nuclide ID's to each material

// Randomizes the concentrations of all nuclides in a variety of materials


// --- from Simulation.cu ---

////////////////////////////////////////////////////////////////////////////////////
// BASELINE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////
// All "baseline" code is at the top of this file. The baseline code is a simple
// implementation of the algorithm, with only minor CPU optimizations in place.
// Following these functions are a number of optimized variants,
// which each deploy a different combination of optimizations strategies. By
// default, XSBench will only run the baseline implementation. Optimized variants
// are not yet implemented in this CUDA port.
////////////////////////////////////////////////////////////////////////////////////


// run the simulation on a host for validation
unsigned long long
run_event_based_simulation(Inputs in, SimulationData SD, int mype)
{
  if(mype==0) printf("Beginning event based simulation on the host for verification...\n");

  int * verification_h = (int *) malloc(in.lookups * sizeof(int));

  // These two are a bit of a hack. Sometimes they are empty buffers (if using hash or nuclide
  // grid methods). OpenCL will throw an example when we try to create an empty buffer. So, we
  // will just allocate some memory for them and move them as normal. The rest of our code
  // won't actually use them if they aren't needed, so this is safe. Probably a cleaner way
  // of doing this.


  lookup_reference (
      SD.num_nucs, SD.concs, SD.mats,
      SD.nuclide_grid, verification_h, SD.unionized_energy_array,
      SD.index_grid, in.lookups, in.n_isotopes, in.n_gridpoints,
      in.grid_type, in.hash_bins, SD.max_num_nucs );

  // Host reduces the verification array
  unsigned long long verification_scalar = 0;
  for( int i = 0; i < in.lookups; i++ )
    verification_scalar += verification_h[i];

  if( SD.length_unionized_energy_array == 0 ) free(SD.unionized_energy_array);
  if( SD.length_index_grid == 0 ) free(SD.index_grid);
  free(verification_h);

  return verification_scalar;
}

unsigned long long
run_event_based_simulation(Inputs in, SimulationData SD,
                           int mype, double *kernel_time)
{

  ////////////////////////////////////////////////////////////////////////////////
  // SUMMARY: Simulation Data Structure Manifest for "SD" Object
  // Here we list all heap arrays (and lengths) in SD that would need to be
  // offloaded manually if using an accelerator with a seperate memory space
  ////////////////////////////////////////////////////////////////////////////////
  // int * num_nucs;                     // Length = length_num_nucs;
  // double * concs;                     // Length = length_concs
  // int * mats;                         // Length = length_mats
  // double * unionized_energy_array;    // Length = length_unionized_energy_array
  // int * index_grid;                   // Length = length_index_grid
  // NuclideGridPoint * nuclide_grid;    // Length = length_nuclide_grid
  //
  // Note: "unionized_energy_array" and "index_grid" can be of zero length
  //        depending on lookup method.
  //
  // Note: "Lengths" are given as the number of objects in the array, not the
  //       number of bytes.
  ////////////////////////////////////////////////////////////////////////////////

  if(mype==0) printf("Beginning event based simulation...\n");

  // Let's create an extra verification array to reduce manually later on
  if( mype == 0 )
     printf("Allocating an additional %.1lf MB of memory for verification arrays...\n",
            in.lookups * sizeof(int) /1024.0/1024.0);

  int * verification_h = (int *) malloc(in.lookups * sizeof(int));

  cudaDeviceProp devProp;
  cudaGetDeviceProperties(&devProp, 0);
  if(mype == 0 ) printf("Running on: %s\n", devProp.name);
  if(mype == 0 ) printf("Initializing device buffers and JIT compiling kernel...\n");

  ////////////////////////////////////////////////////////////////////////////////
  // Create Device Buffers
  ////////////////////////////////////////////////////////////////////////////////

  int *verification_d = nullptr;
  int *mats_d = nullptr ;
  int *num_nucs_d = nullptr;
  double *concs_d = nullptr;
  NuclideGridPoint *nuclide_grid_d = nullptr;

  //buffer<int, 1> num_nucs_d(SD.num_nucs,SD.length_num_nucs);

  //buffer<double, 1> concs_d(SD.concs, SD.length_concs);

  //buffer<int, 1> mats_d(SD.mats, SD.length_mats);

  //buffer<NuclideGridPoint, 1> nuclide_grid_d(SD.nuclide_grid, SD.length_nuclide_grid);

  //buffer<int, 1> verification_d(verification_h, in.lookups);

  // These two are a bit of a hack. Sometimes they are empty buffers (if using hash or nuclide
  // grid methods). OpenCL will throw an example when we try to create an empty buffer. So, we
  // will just allocate some memory for them and move them as normal. The rest of our code
  // won't actually use them if they aren't needed, so this is safe. Probably a cleaner way
  // of doing this.
  //buffer<double,1> unionized_energy_array_d(SD.unionized_energy_array, SD.length_unionized_energy_array);
  double *unionized_energy_array_d = nullptr;


  //buffer<int, 1> index_grid_d(SD.index_grid, (unsigned long long ) SD.length_index_grid);
  int *index_grid_d = nullptr;

  ////////////////////////////////////////////////////////////////////////////////
  // Define Device Kernel
  ////////////////////////////////////////////////////////////////////////////////
  dim3 grids  ((in.lookups + 255) / 256);
  dim3 blocks (256);

  double kstart = get_time();


  double kstop = get_time();
  *kernel_time = (kstop - kstart) / in.kernel_repeat;

  // Host reduces the verification array
  unsigned long long verification_scalar = 0;
  for( int i = 0; i < in.lookups; i++ )
    verification_scalar += verification_h[i];

  if( SD.length_unionized_energy_array == 0 ) free(SD.unionized_energy_array);
  if( SD.length_index_grid == 0 ) free(SD.index_grid);
  free(verification_h);

  return verification_scalar;
}

// binary search for energy on unionized energy grid
// returns lower index
template <class T>

// Calculates the microscopic cross section for a given nuclide & energy
template <class Double_Type, class Int_Type, class NGP_Type>

// Calculates macroscopic cross section based on a given material & energy
template <class Double_Type, class Int_Type, class NGP_Type, class E_GRID_TYPE, class INDEX_TYPE>

// picks a material based on a probabilistic distribution




// --- from XSutils.cu ---






// --- from io.cu ---

#ifdef MPI
#include<mpi.h>
#endif

// Prints Section titles in center of 80 char terminal




// Prints comma separated integers - for ease of reading






// --- from XSbench_header.h ---
#ifndef __XSBENCH_HEADER_H__
#define __XSBENCH_HEADER_H__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <assert.h>
#include <stdint.h>
#include <chrono> 

// Papi Header
#ifdef PAPI
#include "papi.h"
#endif

// Grid types
#define UNIONIZED 0
#define NUCLIDE 1
#define HASH 2

// Simulation types
#define HISTORY_BASED 1
#define EVENT_BASED 2

// Binary Mode Type
#define NONE 0
#define READ 1
#define WRITE 2

// Starting Seed
#define STARTING_SEED 1070

// Specific to AMD and NVIDIA compilers
#if !defined (__NVCC__) && !defined(__HIPCC__)
#define #define #endif

// Structures
typedef struct{
  double energy;
  double total_xs;
  double elastic_xs;
  double absorbtion_xs;
  double fission_xs;
  double nu_fission_xs;
} NuclideGridPoint;

typedef struct{
  int nthreads;
  long n_isotopes;
  long n_gridpoints;
  int lookups;
  char * HM;
  int grid_type; // 0: Unionized Grid (default)    1: Nuclide Grid
  int hash_bins;
  int particles;
  int simulation_method;
  int binary_mode;
  int kernel_id;
  int kernel_repeat;
} Inputs;

typedef struct{
  int * num_nucs;                     // Length = length_num_nucs;
  double * concs;                     // Length = length_concs
  int * mats;                         // Length = length_mats
  double * unionized_energy_array;    // Length = length_unionized_energy_array
  int * index_grid;                   // Length = length_index_grid
  NuclideGridPoint * nuclide_grid;    // Length = length_nuclide_grid
  long length_num_nucs;
  long length_concs;
  long length_mats;
  long length_unionized_energy_array;
  long length_index_grid;
  long length_nuclide_grid;
  int max_num_nucs;
  double * p_energy_samples;
  long length_p_energy_samples;
  int * mat_samples;
  long length_mat_samples;
} SimulationData;

// io.c
void logo(int version);
void center_print(const char *s, int width);
void border_print(void);
void fancy_int(long a);
Inputs read_CLI( int argc, char * argv[] );
void print_CLI_error(void);
void print_inputs(Inputs in, int nprocs, int version);
int print_results( Inputs in, int mype, double runtime, int nprocs,
                   unsigned long long *vhash, double time );
void binary_write( Inputs in, SimulationData SD );
SimulationData binary_read( Inputs in );

// Simulation.c
unsigned long long run_event_based_simulation(Inputs in, SimulationData SD, int mype, double * kernel_init_time);
unsigned long long run_event_based_simulation(Inputs in, SimulationData SD, int mype);

int pick_mat(unsigned long * seed);
double LCG_random_double(uint64_t * seed);
uint64_t fast_forward_LCG(uint64_t seed, uint64_t n);
template <class T>
long grid_search( long n, double quarry, T A);
template <class Double_Type, class Int_Type, class NGP_Type>
void calculate_micro_xs(   double p_energy, int nuc, long n_isotopes,
    long n_gridpoints,
    Double_Type  egrid, Int_Type  index_data,
    NGP_Type  nuclide_grids,
    long idx, double *  xs_vector, int grid_type, int hash_bins );
template <class Double_Type, class Int_Type, class NGP_Type, class E_GRID_TYPE, class INDEX_TYPE>
void calculate_macro_xs( double p_energy, int mat, long n_isotopes,
    long n_gridpoints, Int_Type  num_nucs,
    Double_Type  concs,
    E_GRID_TYPE  egrid, INDEX_TYPE  index_data,
    NGP_Type  nuclide_grids,
    Int_Type  mats,
    double * macro_xs_vector, int grid_type, int hash_bins, int max_num_nucs );

// GridInit.c
SimulationData grid_init_do_not_profile( Inputs in, int mype );

// XSutils.c
int NGP_compare( const void * a, const void * b );
int double_compare(const void * a, const void * b);
size_t estimate_mem_usage( Inputs in );
double get_time(void);

// Materials.c
int * load_num_nucs(long n_isotopes);
int * load_mats( int * num_nucs, long n_isotopes, int * max_num_nucs );
double * load_concs( int * num_nucs, int max_num_nucs );

// binary search for energy on nuclide energy grid
// This funciton is defined in the header, as it is also used by the
// initialization region of the program.
template <class T>
long grid_search_nuclide( long n, double quarry, T A, long low, long high)
{
  long lowerLimit = low;
  long upperLimit = high;
  long examinationPoint;
  long length = upperLimit - lowerLimit;

  while( length > 1 )
  {
    examinationPoint = lowerLimit + ( length / 2 );

    if( A[examinationPoint].energy > quarry )
      upperLimit = examinationPoint;
    else
      lowerLimit = examinationPoint;

    length = upperLimit - lowerLimit;
  }

  return lowerLimit;
}
#endif
