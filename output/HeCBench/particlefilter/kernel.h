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
#include <string.h>
#include <limits.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <float.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>

#define BLOCK_X 16
#define BLOCK_Y 16
#define PI 3.1415926535897932f
#define A 1103515245
#define C 12345
#define M INT_MAX
#define SCALE_FACTOR 300.0f

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 256
#endif


#ifndef FLT_MAX
#define FLT_MAX 3.40282347e+38
#endif

/*****************************
 * Returns a long int representing the time
 *****************************/

/* Returns the number of seconds elapsed between the two specified times */

/**
 * Generates a uniformly distributed random number using the provided seed and GCC's settings for the Linear Congruential Generator (LCG)
 * @see http://en.wikipedia.org/wiki/Linear_congruential_generator
 * @note This function is thread-safe
 * @param seed The seed array
 * @param index The specific index of the seed to be advanced
 * @return a uniformly distributed number [0, 1)
 */

/**
 * Generates a normally distributed random number using the Box-Muller transformation
 * @note This function is thread-safe
 * @param seed The seed array
 * @param index The specific index of the seed to be advanced
 * @return a float representing random number generated using the Box-Muller algorithm
 * @see http://en.wikipedia.org/wiki/Normal_distribution, section computing value for normal random distribution
 */

/**
 * Takes in a float and returns an integer that approximates to that float
 * @return if the mantissa < .5 => return value < input value; else return value > input value
 */

/**
 * Set values of the 3D array to a newValue if that value is equal to the testValue
 * @param testValue The value to be replaced
 * @param newValue The value to replace testValue with
 * @param array3D The image vector
 * @param dimX The x dimension of the frame
 * @param dimY The y dimension of the frame
 * @param dimZ The number of frames
 */

/**
 * Sets values of 3D matrix using randomly generated numbers from a normal distribution
 * @param array3D The video to be modified
 * @param dimX The x dimension of the frame
 * @param dimY The y dimension of the frame
 * @param dimZ The number of frames
 * @param seed The seed array
 */

/**
 * Fills a radius x radius matrix representing the disk
 * @param disk The pointer to the disk to be made
 * @param radius  The radius of the disk to be made
 */

/**
 * Dilates the provided video
 * @param matrix The video to be dilated
 * @param posX The x location of the pixel to be dilated
 * @param posY The y location of the pixel to be dilated
 * @param poxZ The z location of the pixel to be dilated
 * @param dimX The x dimension of the frame
 * @param dimY The y dimension of the frame
 * @param dimZ The number of frames
 * @param error The error radius
 */

/**
 * Dilates the target matrix using the radius as a guide
 * @param matrix The reference matrix
 * @param dimX The x dimension of the video
 * @param dimY The y dimension of the video
 * @param dimZ The z dimension of the video
 * @param error The error radius to be dilated
 * @param newMatrix The target matrix
 */

/**
 * Fills a 2D array describing the offsets of the disk object
 * @param se The disk object
 * @param numOnes The number of ones in the disk
 * @param neighbors The array that will contain the offsets
 * @param radius The radius used for dilation
 */

/**
 * The synthetic video sequence we will work with here is composed of a
 * single moving object, circular in shape (fixed radius)
 * The motion here is a linear motion
 * the foreground intensity and the backgrounf intensity is known
 * the image is corrupted with zero mean Gaussian noise
 * @param I The video itsef
 * @param IszX The x dimension of the video
 * @param IszY The y dimension of the video
 * @param Nfr The number of frames of the video
 * @param seed The seed array used for number generation
 */

/**
 * Finds the first element in the CDF that is greater than or equal to the provided value and returns that index
 * @note This function uses sequential search
 * @param CDF The CDF
 * @param lengthCDF The length of CDF
 * @param value The value to be found
 * @return The index of value in the CDF; if value is never found, returns the last index
 */

/**
 * The implementation of the particle filter using OpenMP for many frames
 * @see http://openmp.org/wp/
 * @note This function is designed to work with a video of several frames. In addition, it references a provided MATLAB function which takes the video, the objxy matrix and the x and y arrays as arguments and returns the likelihoods
 * @param I The video to be run
 * @param IszX The x dimension of the video
 * @param IszY The y dimension of the video
 * @param Nfr The number of frames
 * @param seed The seed array used for random number generation
 * @param Nparticles The number of particles to be used
 */



// --- from kernel_find_index.h ---
void
kernel_find_index (
    const float* arrayX,
    const float* arrayY,
    const float* CDF,
    const float* u,
          float* xj,
          float* yj,
    const int Nparticles)
{
  int i = BLOCK_DIM_X * _bid_x + _tid_x;
  if(i < Nparticles){
    int index = -1;
    int x;

    for(x = 0; x < Nparticles; x++){
      if(CDF[x] >= u[i]){
        index = x;
        break;
      }
    }
    if(index == -1){
      index = Nparticles-1;
    }

    xj[i] = arrayX[index];
    yj[i] = arrayY[index];
  }
}


// --- from kernel_likelihood.h ---
void
kernel_likelihood (
    float* arrayX, 
    float* arrayY, 
    const float* xj,
    const float* yj,
    int* ind,
    const int* objxy,
    float* likelihood,
    const unsigned char* I,
    float* weights,
    int* seed,
    float* partial_sums,
    const int Nparticles,
    const int countOnes,
    const int IszY,
    const int Nfr,
    const int k,
    const int max_size)
{
  float weights_local[BLOCK_SIZE];

  int block_id = _bid_x; 
  int thread_id = _tid_x;
  int i = block_id * BLOCK_DIM_X + thread_id;
  int y;
  int indX, indY;
  float u, v;

  if(i < Nparticles){
    arrayX[i] = xj[i]; 
    arrayY[i] = yj[i]; 

    weights[i] = 1.0f / ((float) (Nparticles)); 
    seed[i] = (A*seed[i] + C) % M;
    u = fabsf(seed[i]/((float)M));
    seed[i] = (A*seed[i] + C) % M;
    v = fabsf(seed[i]/((float)M));
    arrayX[i] += 1.0f + 5.0f*(sqrtf(-2.0f*logf(u))*cosf(2.0f*PI*v));

    seed[i] = (A*seed[i] + C) % M;
    u = fabsf(seed[i]/((float)M));
    seed[i] = (A*seed[i] + C) % M;
    v = fabsf(seed[i]/((float)M));
    arrayY[i] += -2.0f + 2.0f*(sqrtf(-2.0f*logf(u))*cosf(2.0f*PI*v));
  }

  if(i < Nparticles)
  {
    for(y = 0; y < countOnes; y++){

      int iX = arrayX[i];
      int iY = arrayY[i];
      int rnd_iX = (arrayX[i] - iX) < .5f ? iX : iX++;
      int rnd_iY = (arrayY[i] - iY) < .5f ? iY : iY++;
      indX = rnd_iX + objxy[y*2 + 1];
      indY = rnd_iY + objxy[y*2];

      ind[i*countOnes + y] = abs(indX*IszY*Nfr + indY*Nfr + k);
      if(ind[i*countOnes + y] >= max_size)
        ind[i*countOnes + y] = 0;
    }
    float likelihoodSum = 0.0f;
    for(int x = 0; x < countOnes; x++)
      likelihoodSum += ((I[ind[i*countOnes + x]] - 100) * (I[ind[i*countOnes + x]] - 100) -
          (I[ind[i*countOnes + x]] - 228) * (I[ind[i*countOnes + x]] - 228)) / 50.0f;
    likelihood[i] = likelihoodSum/countOnes-SCALE_FACTOR;

    weights[i] = weights[i] * expf(likelihood[i]); //Donnie Newell - added the missing exponential function call

  }

  weights_local[thread_id] = (i < Nparticles) ? weights[i] : 0.f;

  for(unsigned int s=BLOCK_SIZE/2; s>0; s>>=1)
  {
    if(thread_id < s)
    {
      weights_local[thread_id] += weights_local[thread_id + s];
    }
  }
  if(thread_id == 0)
  {
    partial_sums[block_id] = weights_local[0];
  }
}


// --- from kernel_normalize_weights.h ---
void
kernel_normalize_weights (
    float*  weights,
    const float*  partial_sums,
    float*  CDF,
    float*  u,
    int*  seed,
    const int Nparticles )
{
  float u1;
  float sumWeights;
  int local_id = _tid_x;
  int i = BLOCK_DIM_X * _bid_x + _tid_x;
  if(0 == local_id)
    sumWeights = partial_sums[0];
  if(i < Nparticles) {
    weights[i] = weights[i]/sumWeights;
  }
  if(i == 0) {
    CDF[0] = weights[0];
    for(int x = 1; x < Nparticles; x++){
      CDF[x] = weights[x] + CDF[x-1];
    }

    seed[i] = (A*seed[i] + C) % M;
    float p = fabsf(seed[i]/((float)M));
    seed[i] = (A*seed[i] + C) % M;
    float q = fabsf(seed[i]/((float)M));
    u[0] = (1.0f/((float)(Nparticles))) * 
      (sqrtf(-2.0f*log(p))*cosf(2.0f*PI*q));
    // do this to allow all threads in all blocks to use the same u1
  }
  if(0 == local_id)
    u1 = u[0];
  if(i < Nparticles)
  {
    u[i] = u1 + i/((float)(Nparticles));
  }
}


// --- from kernel_sum.h ---
void
kernel_sum (float* partial_sums, const int Nparticles)
{
  int x;
  float sum = 0;
  int num_blocks = (Nparticles + BLOCK_SIZE - 1) / BLOCK_SIZE;
  for (x = 0; x < num_blocks; x++) {
    sum += partial_sums[x];
  }
  partial_sums[0] = sum;
}

