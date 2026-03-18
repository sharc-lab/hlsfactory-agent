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

// --- from bucketsort.cu ---
#include <fcntl.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

// CUDA kernels

////////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////////
void calcPivotPoints(float *histogram, int histosize, int listsize,
    int divisions, float min, float max, float *pivotPoints,
    float histo_width);

////////////////////////////////////////////////////////////////////////////////
// Given the input array of floats and the min and max of the distribution,
// sort the elements into float4 aligned buckets of roughly equal size
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Given a histogram of the list, figure out suitable pivotpoints that divide
// the list into approximately listsize/divisions elements each
////////////////////////////////////////////////////////////////////////////////


// --- from hybridsort.cu ---

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <fcntl.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#define TIMER 

////////////////////////////////////////////////////////////////////////////////

// Use a static data size for simplicity
//
#define SIZE (50000000)

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
float4* runMergeSort(int listsize, int divisions,
    float4 *d_origList, float4 *d_resultList,
    int *sizes, int *nullElements,
    unsigned int *origOffsets);




// --- from mergesort.cu ---
#include "hip/hip_runtime.h"
#include <fcntl.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////
#define BLOCKSIZE  256
#define ROW_LENGTH  BLOCKSIZE * 4
#define ROWS    4096

////////////////////////////////////////////////////////////////////////////////
// The mergesort algorithm
////////////////////////////////////////////////////////////////////////////////





// the kernel calls the functions defined above




// --- from bucketsort.h ---
#ifndef __BUCKETSORT
#define __BUCKETSORT

#include <hip/hip_runtime.h>

#define LOG_DIVISIONS	10
#define DIVISIONS		(1 << LOG_DIVISIONS)

#define BUCKET_WARP_LOG_SIZE	5
#define BUCKET_WARP_N			1

#ifdef BUCKET_WG_SIZE_1
#define BUCKET_THREAD_N BUCKET_WG_SIZE_1
#else
#define BUCKET_THREAD_N			(BUCKET_WARP_N << BUCKET_WARP_LOG_SIZE)
#endif
#define BUCKET_BLOCK_MEMORY		(DIVISIONS * BUCKET_WARP_N)
#define BUCKET_BAND				128

#define HISTOGRAM_BIN_COUNT  1024
#define HISTOGRAM_BLOCK_MEMORY  (3 * HISTOGRAM_BIN_COUNT)
#define IMUL(a, b) __mul24(a, b)

void bucketSort(float *d_input, float *d_output, int listsize,
				int *sizes, int *nullElements, float minimum, float maximum,
				unsigned int *origOffsets);
double getBucketTime();

#endif


// --- from kernel_bucketcount.h ---
  void
bucketcount (const float* input , 
    int* indice,
    unsigned int* prefixoffsets,
    const float* pivotpoints,
    const int listsize)
{
  unsigned int s_offset[BUCKET_BLOCK_MEMORY]; 

  const int gid = _bid_x * BLOCK_DIM_X + _tid_x;
  const int lid = _tid_x;
  const int gsize = GRID_DIM_X * BLOCK_DIM_X;
  const int lsize = BLOCK_DIM_X;
  const int warpBase = (lid >> BUCKET_WARP_LOG_SIZE) * DIVISIONS;
  const int numThreads = gsize;

  for (int i = lid; i < BUCKET_BLOCK_MEMORY; i += lsize)
    s_offset[i] = 0;

  for (int tid = gid; tid < listsize; tid += numThreads) {
    float elem = input[tid];

    int idx  = DIVISIONS/2 - 1;
    int jump = DIVISIONS/4;
    float piv = pivotpoints[idx]; //s_pivotpoints[idx];

    while(jump >= 1){
      idx = (elem < piv) ? (idx - jump) : (idx + jump);
      piv = pivotpoints[idx]; //s_pivotpoints[idx];
      jump /= 2;
    }
    idx = (elem < piv) ? idx : (idx + 1);

    indice[tid] = 
      ((s_offset[warpBase+idx] += 1U) << LOG_DIVISIONS)  + idx;
  }

  int prefixBase = _bid_x * BUCKET_BLOCK_MEMORY;

  for (int i = lid; i < BUCKET_BLOCK_MEMORY; i += lsize)
    prefixoffsets[prefixBase + i] = s_offset[i] & 0x07FFFFFFU;

}



// --- from kernel_bucketprefix.h ---
#include "hip/hip_runtime.h"
void
bucketprefix (
    unsigned int* prefixoffsets,
    unsigned int* offsets,
    int blocks )
{

  const int tid = _bid_x * BLOCK_DIM_X + _tid_x;
  const int size = blocks * BUCKET_BLOCK_MEMORY;
  int sum = 0;

  for (int i = tid; i < size; i += DIVISIONS) {
    int x = prefixoffsets[i];
    prefixoffsets[i] = sum;
    sum += x;
  }
  offsets[tid] = sum;
}



// --- from kernel_bucketsort.h ---
#include "hip/hip_runtime.h"
void
bucketsort (const float* input , 
    const int* indice,
    float* output,
    const unsigned int* prefixoffsets,
    const unsigned int* offsets,
    const int listsize)
{
  const int grp_id = _bid_x;
  const int gid = _bid_x * BLOCK_DIM_X + _tid_x;
  const int lid = _tid_x;
  const int gsize = GRID_DIM_X * BLOCK_DIM_X;
  const int lsize = BLOCK_DIM_X;

  unsigned int s_offset[BUCKET_BLOCK_MEMORY]; 

  int prefixBase = grp_id * BUCKET_BLOCK_MEMORY;
  const int warpBase = (lid >> BUCKET_WARP_LOG_SIZE) * DIVISIONS;
  const int numThreads = gsize;

  for (int i = lid; i < BUCKET_BLOCK_MEMORY; i += lsize){
    s_offset[i] = offsets[i & (DIVISIONS - 1)] + prefixoffsets[prefixBase + i];
  }

  for (int tid = gid; tid < listsize; tid += numThreads){
    float elem = input[tid];
    int id = indice[tid];
    output[s_offset[warpBase + (id & (DIVISIONS - 1))] + (id >>  LOG_DIVISIONS)] = elem;
  }
}



// --- from kernel_histogram.h ---
#include "hip/hip_runtime.h"
////////////////////////////////////////////////////////////////////////////////
// Main computation pass: compute per-workgroup partial histograms
////////////////////////////////////////////////////////////////////////////////
  void
histogram1024 ( unsigned int* histoOutput, 
    const float* histoInput,
    const int listsize,
    const float minimum,
    const float maximum)
{
  unsigned int s_Hist[HISTOGRAM_BLOCK_MEMORY]; 

  const int gid = _bid_x * BLOCK_DIM_X + _tid_x;
  const int lid = _tid_x;
  const int gsize = GRID_DIM_X * BLOCK_DIM_X;
  const int lsize = BLOCK_DIM_X;

  //Per-warp substorage storage
  int mulBase = (lid >> BUCKET_WARP_LOG_SIZE);
  const int warpBase = IMUL(mulBase, HISTOGRAM_BIN_COUNT);

  //Clear shared memory storage for current threadblock before processing
  for(uint i = lid; i < HISTOGRAM_BLOCK_MEMORY; i+=lsize) {
    s_Hist[i] = 0;
  }

  //Read through the entire input buffer, build per-warp histograms
  for(int pos = gid; pos < listsize; pos += gsize) {
    uint data4 = ((histoInput[pos] - minimum)/(maximum - minimum)) * HISTOGRAM_BIN_COUNT;

    atomicAdd(&s_Hist[warpBase+(data4 & 0x3FFU)], 1U);
  }

  //Per-block histogram reduction

  for(int pos = lid; pos < HISTOGRAM_BIN_COUNT; pos += lsize){
    uint sum = 0;
    for(int i = 0; i < HISTOGRAM_BLOCK_MEMORY; i+= HISTOGRAM_BIN_COUNT){ 
      sum += s_Hist[pos + i] & 0x07FFFFFFU;
    }
    (histoOutput[pos] += sum);
  }
}



// --- from kernel_mergeSortPass.h ---
#include "hip/hip_runtime.h"
void
mergeSortPass (const float4* input, 
    float4* result,
    const int* constStartAddr,
    const int threadsPerDiv,
    const int nrElems,
    const int size)
{

  const int gid = _bid_x * BLOCK_DIM_X + _tid_x;
  // The division to work on
  int division = gid / threadsPerDiv;
  if(division >= DIVISIONS) return;
  // The block within the division
  int int_gid = gid - division * threadsPerDiv;
  int Astart = constStartAddr[division] + int_gid * nrElems;

  int Bstart = Astart + nrElems/2;
  float4* resStart= &(result[Astart]);

  if(Astart >= constStartAddr[division + 1])
    return;
  if(Bstart >= constStartAddr[division + 1]){
    for(int i=0; i<(constStartAddr[division + 1] - Astart); i++)
    {
      resStart[i] = input[Astart + i];
    }
    return;
  }

  int aidx = 0;
  int bidx = 0;
  int outidx = 0;
  float4 a, b;
  a = input[Astart + aidx];
  b = input[Bstart + bidx];

  while(true)//aidx < nrElems/2)// || (bidx < nrElems/2  && (Bstart + bidx < constEndAddr[division])))
  {
    /**
     * For some reason, it's faster to do the texture fetches here than
     * after the merge
     */
    float4 nextA = input[Astart + aidx + 1];
    float4 nextB = (Bstart + bidx + 1 >= size) ? 
                   float4(0.f, 0.f, 0.f, 0.f) : input[Bstart + bidx + 1];

    float4 na = getLowest(a,b);
    float4 nb = getHighest(a,b);
    a = sortElem(na);
    b = sortElem(nb);
    // Now, a contains the lowest four elements, sorted
    resStart[outidx++] = a;

    bool elemsLeftInA;
    bool elemsLeftInB;

    elemsLeftInA = (aidx + 1 < nrElems/2); // Astart + aidx + 1 is allways less than division border
    elemsLeftInB = (bidx + 1 < nrElems/2) && (Bstart + bidx + 1 < constStartAddr[division + 1]);

    if(elemsLeftInA){
      if(elemsLeftInB){
        float nextA_t = nextA.x;
        float nextB_t = nextB.x;
        if(nextA_t < nextB_t) { aidx += 1; a = nextA; }
        else { bidx += 1;  a = nextB; }
      }
      else {
        aidx += 1; a = nextA;
      }
    }
    else {
      if(elemsLeftInB){
        bidx += 1;  a = nextB;
      }
      else {
        break;
      }
    }

  }
  resStart[outidx++] = b;
}



// --- from mergesort.h ---
#ifndef __MERGESORT
#define __MERGESORT


float4* runMergeSort(int listsize, int divisions,
					 float4 *d_origList, float4 *d_resultList,
					 int *sizes, int *nullElements,
					 unsigned int *origOffsets);
#endif
