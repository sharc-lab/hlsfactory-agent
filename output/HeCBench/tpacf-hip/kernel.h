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

// --- from ACF_kernel.cu ---
/*
   Illinois Open Source License

   University of Illinois/NCSA
   Open Source License

   Copyright © 2009,    University of Illinois.  All rights reserved.

   Developed by: 
   Innovative Systems Lab
   National Center for Supercomputing Applications
http://www.ncsa.uiuc.edu/AboutUs/Directorates/ISL.html

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal with the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimers.

 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimers in the documentation and/or other materials provided with the distribution.

 * Neither the names of Innovative Systems Lab and National Center for Supercomputing Applications, nor the names of its contributors may be used to endorse or promote products derived from this Software without specific prior written permission.

 THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
 */

// Angular correlation function kernel
// Takes two sets of cartesians in g_idata1, g_idata2,
// computes dot products for all pairs, uses waterfall search
// to determine the appropriate bin for each dot product,
// and outputs bins in g_odata (packed 4 bins to 1 unsigned int)

// The problem is treated as a grid of dot products.
// Each thread block has 128 threads, and calculates the dot
// products for a 128x128 sub-grid.

#ifndef _ACF_KERNEL_H_
#define _ACF_KERNEL_H_

#define LOG2_GRID_SIZE 14

double binbounds[NUMBINS-1];

// Similar to ACF kernel, but takes advantage of symmetry to cut computations down by half.
// Obviously, due to symmetry, it needs only one input set.


#endif


// --- from compute.cu ---
/*
   Illinois Open Source License

   University of Illinois/NCSA
   Open Source License

   Copyright © 2009,    University of Illinois.  All rights reserved.

   Developed by: 
   Innovative Systems Lab
   National Center for Supercomputing Applications
http://www.ncsa.uiuc.edu/AboutUs/Directorates/ISL.html

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal with the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimers.

 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimers in the documentation and/or other materials provided with the distribution.

 * Neither the names of Innovative Systems Lab and National Center for Supercomputing Applications, nor the names of its contributors may be used to endorse or promote products derived from this Software without specific prior written permission.

 THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
 */

#ifndef _GPU_COMPUTE_H_
#define _GPU_COMPUTE_H_

#include <hip/hip_runtime.h>
#include <string.h>
#include <sys/time.h>
#include "kernel.h"
#include "args.h"

#define TDIFF(ts, te) (te.tv_sec - ts.tv_sec + (te.tv_usec - ts.tv_usec) * 1e-6)

#define GRID_SIZE  (1 << LOG2_GRID_SIZE)

const dim3 grid(128, 128, 1);
const dim3 threads(128, 1, 1);

// Device-side data storage
cartesian d_idata1;
cartesian d_idata2;
unsigned int* d_odata1;

// Host-side data storage
cartesian h_idata1;
cartesian h_idata2;

// Performance
struct timeval t1, t0;
float t_Compute = 0.0f;

// Writes bin boundaries to GPU constant memory.

// Used to compute DD or RR, takes advantage of symmetry to reduce number of dot products and waterfall searches
// required by half. Unfortunately, due to the limitations of the histogram kernel, every element of d_odata still
// represents a histogram bin assignment; consequently the histogram kernel does just as much work in tileComputeSymm
// as it does in tileCompute.

// type: type = 0 corresponds to DD, type = 1 corresponds to RR.
// size: Number of elements in data or random set (dependent upon type)
// njk: Number of jackknives
// jkSizes: List of jackknife sizes, in order
// nBins: Number of histogram bins
// histo: The function outputs by adding on to this histogram
// stream: CUDA stream

// Used to compute DR. 
// dataSize: Size of data set
// randomSize: Size of random set
// All else: See descriptions in tileComputeSymm

// Computes histograms and writes to DDs, DRs, RRs. These must be compiled by the host program;
// the function outputs njk sub-histograms for each, the sum of which is the full histogram.
// Note that data should be sorted according to jackknife; otherwise results will be meaningless.

// dataName: File name of data points file.
// randomNames: File name stem of random points file.
// nr: Number of random files. Random files are assumed to be of the form randomNames.i where 1 <= i <= nr.
// dataSize: Number of elements to read from data points file.
// randomSize: Number of elements to read from each random points file.
// njk: Number of jackknives.
// jkSizes: Ordered list of jackknife sizes. Each size must be a multiple of 4 currently.
// nBins: Number of histogram bins.
// zeroBin: Index of bin which contains 0.0f: Necessary to correct for padding
// DDs, DRs, RRs: Output subhistogram lists.


#endif



// --- from histogram_kernel.cu ---
/*
   Illinois Open Source License

   University of Illinois/NCSA
   Open Source License

   Copyright © 2009,    University of Illinois.  All rights reserved.

   Developed by: 
   Innovative Systems Lab
   National Center for Supercomputing Applications
http://www.ncsa.uiuc.edu/AboutUs/Directorates/ISL.html

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal with the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimers.

 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimers in the documentation and/or other materials provided with the distribution.

 * Neither the names of Innovative Systems Lab and National Center for Supercomputing Applications, nor the names of its contributors may be used to endorse or promote products derived from this Software without specific prior written permission.

 THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
 */

#ifndef _HISTOGRAM_KERNEL_H_
#define _HISTOGRAM_KERNEL_H_

// 64 bin histogram kernel, based on nVidia whitepaper.

// NUMBINS: Number of bins, should be <= 64 on G8x hardware due to memory.
// NUMTHREADS: Number of threads per block, should be 128-256 and a multiple of 32.
// MAXNUMBLOCKS: Max number of blocks (for memory limitations).
// MAXBLOCKSEND: Merge will reduce the number of blocks to this.
// DATAPERBLOCK: Number of 32 bit words processed by each block.
//         (Note: 63 is to prevent overrun in the 1 byte counters.)
// MEMPERBLOCK: Shared memory used to store per-thread sub-histograms.
// HISTOSIZE: Size of a histogram.
#define   NUMTHREADS    128
#define   MAXNUMBLOCKS  16384
#define   MAXBLOCKSEND  32
#define   DATAPERBLOCK  (NUMTHREADS * 63)
#define   MEMPERBLOCK   (NUMTHREADS * NUMBINS)
#define   HISTOSIZE     (NUMBINS * sizeof(unsigned int))

// Device-side and Host-side memory for output.
unsigned int *d_odata;
unsigned int *h_odata;

// Helper function, integer division with rounding up.
int iDivUp(int a, int b);
// Computes per-block sub-histograms and stores them in g_odata
void histoKernel(unsigned int* g_odata, unsigned int* g_idata, int size);
// Compiles per-block sub-histograms into MAXBLOCKSEND sub-histograms.

// Free memory

// Host-side function; returns a NUMBINS bin histogram in h_result based off of d_idata
// IMPORTANT: d_idata is assumed to pack four bin assignments in a 32 bit integer, treating
// the 6 upper bits (0xFC) of each byte as a bin assignment. num is the number of bin assignments,
// not the number of integers in d_idata.


// g_odata are actual unsigned ints; g_idata pack four bin assignments as per above
// size is actually the number of unsigned ints in g_idata now, unlike num above.

// Merges numBlocks per-block sub-histograms into GRID_DIM_X sub-histograms.
// (GRID_DIM_X should be MAXBLOCKSEND for obvious reasons)

#endif
