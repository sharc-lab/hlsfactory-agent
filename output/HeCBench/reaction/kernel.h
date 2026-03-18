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

// --- from kernels.cu ---
/**
 * @brief      Calculate Gray-Scott reaction rate
 */

/**
 * @brief      Calculate second derivative in x direction with periodic boundary conditions
 */

/**
 * @brief      Calculate second derivative in x direction with zero-flux boundary conditions
 */

/**
 * @brief      Calculate second derivative in y direction with periodic boundary conditions
 */

/**
 * @brief      Calculate second derivative in y direction with zero-flux  boundary conditions
 */

/**
 * @brief      Calculate second derivative in z direction with periodic boundary conditions
 */

/**
 * @brief      Calculate second derivative in z direction with zero-flux boundary conditions
 */

/**
 * @brief      Construct the Laplacian for a component 
 */

/**
 * @brief      Perform time-step integration
 *
 * @param      x     pointer to concentration of A
 * @param      y     pointer to concentration of B
 * @param[in]  ddx   diffusion of component A
 * @param[in]  ddy   diffusion of component B
 * @param[in]  drx   reaction of component A
 * @param[in]  dry   reaction of component B
 */


// --- from main.cu ---
#include <chrono>
#include <random>
#include <new>



// --- from util.h ---
static float uniform_dist() {
  static std::mt19937 rng(123);
  static std::uniform_real_distribution<> nd(0.0, 1.0);
  return nd(rng);
}

/**
 * @brief      Build random input
 *
 * @param      a      Concentration of a
 * @param      b      Concentration of b
 * @param[in]  a0     initial value a
 * @param[in]  b0     initial value b
 * @param[in]  ca     central concentration for a
 * @param[in]  cb     central concentration for b
 * @param[in]  delta  perturbation strength
 */
void build_input_central_cube(
    unsigned int ncells, unsigned int mx, unsigned int my, unsigned int mz, 
    float* a, float* b, float a0, float b0, float ca, float cb, float delta) 
{
  // initialize with random data
  for(unsigned int i=0; i < ncells; i++) {
    a[i] = a0 + uniform_dist() * delta;
    b[i] = b0 + uniform_dist() * delta;
  }

  const unsigned int cbsz = 5;
  for(unsigned int z=mz/2-cbsz; z<mz/2+cbsz; z++) {
    for(unsigned int y=my/2-cbsz; y<my/2+cbsz; y++) {
      for(unsigned int x=mx/2-cbsz; x<mx/2+cbsz; x++) {
        a[z * mx * my + y * mx + x] = ca  + uniform_dist() * delta;
        b[z * mx * my + y * mx + x] = cb  + uniform_dist() * delta;
      }
    }
  }
}

// output lowest and highest values
void stats(const float *a, const float *b, unsigned int ncells) {
  float minValueA = 100.0;
  float minValueB = 100.0;
  float maxValueA = 0.0;
  float maxValueB = 0.0;
  for(unsigned int i=0; i<ncells; i++) {
    minValueA = std::min(minValueA, a[i]);
    minValueB = std::min(minValueB, b[i]);
  }

  for(unsigned int i=0; i<ncells; i++) {
    maxValueA = std::max(maxValueA, a[i]);
    maxValueB = std::max(maxValueB, b[i]);
  }
  printf("  Components A | B \n");
  printf("  Min = %12.6f | %12.6f\n", minValueA, minValueB);
  printf("  Max = %12.6f | %12.6f\n", maxValueA, maxValueB);
}
