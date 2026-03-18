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

// --- from Simulation.cu ---
#include <hip/hip_runtime.h>
#include "XSbench_header.h"

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

  hipDeviceProp_t devProp;
  hipGetDeviceProperties(&devProp, 0);
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
  hipMalloc((void**)&num_nucs_d, sizeof(int) * SD.length_num_nucs);
  hipMemcpy(num_nucs_d, SD.num_nucs, sizeof(int) * SD.length_num_nucs, hipMemcpyHostToDevice);

  //buffer<double, 1> concs_d(SD.concs, SD.length_concs);
  hipMalloc((void**)&concs_d, sizeof(double) * SD.length_concs);
  hipMemcpy(concs_d, SD.concs, sizeof(double) * SD.length_concs, hipMemcpyHostToDevice);

  //buffer<int, 1> mats_d(SD.mats, SD.length_mats);
  hipMalloc((void**)&mats_d, sizeof(int) * SD.length_mats);
  hipMemcpy(mats_d, SD.mats, sizeof(int) * SD.length_mats, hipMemcpyHostToDevice);

  //buffer<NuclideGridPoint, 1> nuclide_grid_d(SD.nuclide_grid, SD.length_nuclide_grid);
  hipMalloc((void**)&nuclide_grid_d, sizeof(NuclideGridPoint) * SD.length_nuclide_grid);
  hipMemcpy(nuclide_grid_d, SD.nuclide_grid, sizeof(NuclideGridPoint) * SD.length_nuclide_grid, hipMemcpyHostToDevice);

  //buffer<int, 1> verification_d(verification_h, in.lookups);
  //buffer<double,1> unionized_energy_array_d(SD.unionized_energy_array, SD.length_unionized_energy_array);
  double *unionized_energy_array_d = nullptr;
  hipMalloc((void**)&unionized_energy_array_d, sizeof(double) * SD.length_unionized_energy_array);
  hipMemcpy(unionized_energy_array_d, SD.unionized_energy_array,

  //buffer<int, 1> index_grid_d(SD.index_grid, (unsigned long long ) SD.length_index_grid);
  int *index_grid_d = nullptr;
  hipMalloc((void**)&index_grid_d, sizeof(int) * (unsigned long long)SD.length_index_grid);
  hipMemcpy(index_grid_d, SD.index_grid, sizeof(int) * (unsigned long long )SD.length_index_grid, hipMemcpyHostToDevice);

  hipDeviceSynchronize();

  ////////////////////////////////////////////////////////////////////////////////
  // Define Device Kernel
  ////////////////////////////////////////////////////////////////////////////////
  dim3 grids  ((in.lookups + 255) / 256);
  dim3 blocks (256);

  double kstart = get_time();


  hipDeviceSynchronize();
  double kstop = get_time();
  *kernel_time = (kstop - kstart) / in.kernel_repeat;

  hipMemcpy(verification_h, verification_d, sizeof(int) * in.lookups, hipMemcpyDeviceToHost);

  hipFree(verification_d);
  hipFree(mats_d);
  hipFree(num_nucs_d);
  hipFree(concs_d);
  hipFree(nuclide_grid_d);
  hipFree(unionized_energy_array_d);
  hipFree(index_grid_d);

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


