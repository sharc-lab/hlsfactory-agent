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

// --- from kernel_reference.cu ---
/*
 * Copyright (c) 2016 University of Cordoba and University of Illinois
 * All rights reserved.
 *
 * Developed by:    IMPACT Research Group
 *                  University of Cordoba and University of Illinois
 *                  http://impact.crhc.illinois.edu/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * with the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *      > Redistributions of source code must retain the above copyright notice,
 *        this list of conditions and the following disclaimers.
 *      > Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimers in the
 *        documentation and/or other materials provided with the distribution.
 *      > Neither the names of IMPACT Research Group, University of Cordoba, 
 *        University of Illinois nor the names of its contributors may be used 
 *        to endorse or promote products derived from this Software without 
 *        specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH
 * THE SOFTWARE.
 *
 */

#include <math.h>
#include <thread>
#include <vector>
#include <algorithm>

float gaus[3][3] = {{0.0625, 0.125, 0.0625}, {0.1250, 0.250, 0.1250}, {0.0625, 0.125, 0.0625}};
// Some of the available convolution kernels
int sobx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
int soby[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
// CPU threads--------------------------------------------------------------------------------------


// --- from main.cu ---
/*
 * Copyright (c) 2016 University of Cordoba and University of Illinois
 * All rights reserved.
 *
 * Developed by:    IMPACT Research Group
 *                  University of Cordoba and University of Illinois
 *                  http://impact.crhc.illinois.edu/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * with the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *      > Redistributions of source code must retain the above copyright notice,
 *        this list of conditions and the following disclaimers.
 *      > Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimers in the
 *        documentation and/or other materials provided with the distribution.
 *      > Neither the names of IMPACT Research Group, University of Cordoba, 
 *        University of Illinois nor the names of its contributors may be used 
 *        to endorse or promote products derived from this Software without 
 *        specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH
 * THE SOFTWARE.
 *
 */
#include <unistd.h>
#include <thread>
#include <assert.h>
#include <chrono>

#include "support/partitioner.h"
#include "support/verify.h"

float c_gaus[9] = {0.0625f, 0.125f, 0.0625f, 
                                0.1250f, 0.250f, 0.1250f, 
                                0.0625f, 0.125f, 0.0625f};
int   c_sobx[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
int   c_soby[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

// https://github.com/smskelley/canny-opencl
// Gaussian Kernel
// data: image input data with each pixel taking up 1 byte (8Bit 1Channel)
// out: image output data (8B1C)

// Sobel kernel. Apply sobx and soby separately, then find the sqrt of their
//               squares.
// data:  image input data with each pixel taking up 1 byte (8Bit 1Channel)
// out:   image output data (8B1C)
// theta: angle output data

// Non-maximum Supression Kernel
// data: image input data with each pixel taking up 1 byte (8Bit 1Channel)
// out: image output data (8B1C)
// theta: angle input data

// Hysteresis Threshold Kernel
// data: image input data with each pixel taking up 1 byte (8Bit 1Channel)
// out: image output data (8B1C)

// Params ---------------------------------------------------------------------
struct Params {

  int         device;
  int         n_gpu_threads;
  int         n_threads;
  int         n_warmup;
  int         n_reps;
  float       alpha;
  const char *file_name;
  const char *comparison_file;
  int         display = 0;


};

// Input Data -----------------------------------------------------------------



// --- from kernel.h ---
#ifndef __KERNEL
#define __KERNEL
#include <vector>
#include <algorithm>
#include <string.h>
#ifdef CHAI_OPENCV
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#endif

void run_cpu_threads(unsigned char *buffer0, unsigned char *buffer1, 
		unsigned char *theta, int rows, int cols, int num_threads, int t_index);

#endif
