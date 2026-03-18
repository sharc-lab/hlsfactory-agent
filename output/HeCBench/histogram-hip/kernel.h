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

// --- from histogram_compare.cu ---
/******************************************************************************
 * Copyright (c) 2011-2018, NVIDIA CORPORATION.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the NVIDIA CORPORATION nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#include <stdio.h>
#include <map>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>

// Decode float4 pixel into bins
template <int NUM_BINS, int ACTIVE_CHANNELS>

// Decode uchar4 pixel into bins
template <int NUM_BINS, int ACTIVE_CHANNELS>

// Decode uchar1 pixel into bins
template <int NUM_BINS, int ACTIVE_CHANNELS>

//---------------------------------------------------------------------
// Globals, constants, and type declarations
//---------------------------------------------------------------------

// Ensure printing of CUDA runtime errors to console

bool                    g_verbose = false;  // Whether to display input/output to console
bool                    g_report = false;   // Whether to display a full report in CSV format

struct less_than_value
{
};

//---------------------------------------------------------------------
// Targa (.tga) image file parsing
//---------------------------------------------------------------------

/**
 * TGA image header info
 */
struct TgaHeader
{
    char idlength;
    char colormaptype;
    char datatypecode;
    short colormaporigin;
    short colormaplength;
    char colormapdepth;
    short x_origin;
    short y_origin;
    short width;
    short height;
    char bitsperpixel;
    char imagedescriptor;


};

/**
 * Decode image byte data into pixel
 */

/**
 * Reads a .tga image file
 */

//---------------------------------------------------------------------
// Random image generation
//---------------------------------------------------------------------

/**
 * Generate a random image with specified entropy
 */

//---------------------------------------------------------------------
// Histogram verification
//---------------------------------------------------------------------

// Decode float4 pixel into bins
template <int NUM_BINS, int ACTIVE_CHANNELS>

// Decode uchar4 pixel into bins
template <int NUM_BINS, int ACTIVE_CHANNELS>

// Decode uchar1 pixel into bins
template <int NUM_BINS, int ACTIVE_CHANNELS>

// Compute reference histogram.  Specialized for uchar4
template <
    int         ACTIVE_CHANNELS,
    int         NUM_BINS,
    typename    PixelType>

//---------------------------------------------------------------------
// Test execution
//---------------------------------------------------------------------

/**
 * Run a specific histogram implementation
 */
template <
    int         ACTIVE_CHANNELS,
    int         NUM_BINS,
    typename    PixelType>

/**
 * Evaluate corpus of histogram implementations
 */
template <
    int         NUM_CHANNELS,
    int         ACTIVE_CHANNELS,
    int         NUM_BINS,
    typename    PixelType>

/**
 * Test different problem genres
 */

/**
 * Main
 */


// --- from histogram_gmem_atomics.h ---
/******************************************************************************
 * Copyright (c) 2011-2018, NVIDIA CORPORATION.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the NVIDIA CORPORATION nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

    // First-pass histogram kernel (binning into privatized counters)
    template <
        int         NUM_PARTS,
        int         ACTIVE_CHANNELS,
        int         NUM_BINS,
        typename    PixelType>
    void histogram_gmem_atomics(
        const PixelType *in,
        int width,
        int height,
        unsigned int *out)
    {
        // global position and size
        int x = _bid_x * BLOCK_DIM_X + _tid_x;
        int y = _bid_y * BLOCK_DIM_Y + _tid_y;
        int nx = BLOCK_DIM_X * GRID_DIM_X;
        int ny = BLOCK_DIM_Y * GRID_DIM_Y;

        // threads in workgroup
        int t = _tid_x + _tid_y * BLOCK_DIM_X; // thread index in workgroup, linear in 0..nt-1
        int nt = BLOCK_DIM_X * BLOCK_DIM_Y; // total threads in workgroup

        // group index in 0..ngroups-1
        int g = _bid_x + _bid_y * GRID_DIM_X;

        // initialize global memory
        unsigned int *gmem = out + g * NUM_PARTS;
        for (int i = t; i < ACTIVE_CHANNELS * NUM_BINS; i += nt)
            gmem[i] = 0;

        // process pixels (updates our group's partial histogram in gmem)
        for (int col = x; col < width; col += nx)
        {
            for (int row = y; row < height; row += ny)
            {
                PixelType pixel = in[row * width + col];

                unsigned int bins[ACTIVE_CHANNELS];
                DecodePixel<NUM_BINS>(pixel, bins);

                #pragma unroll
                for (int CHANNEL = 0; CHANNEL < ACTIVE_CHANNELS; ++CHANNEL)
                    atomicAdd(&gmem[(NUM_BINS * CHANNEL) + bins[CHANNEL]], 1);
            }
        }
    }

    // Second pass histogram kernel (accumulation)
    template <
        int         NUM_PARTS,
        int         ACTIVE_CHANNELS,
        int         NUM_BINS>
    void histogram_gmem_accum(
        const unsigned int *in,
        int n,
        unsigned int *out)
    {
        int i = _bid_x * BLOCK_DIM_X + _tid_x;
        if (i > ACTIVE_CHANNELS * NUM_BINS)
            return; // out of range

        unsigned int total = 0;
        for (int j = 0; j < n; j++)
            total += in[i + NUM_PARTS * j];

        out[i] = total;
    }

template <
    int         ACTIVE_CHANNELS,
    int         NUM_BINS,
    typename    PixelType>
double run_gmem_atomics(
    PixelType *d_image,
    int width,
    int height,
    unsigned int *d_hist,
    bool warmup)
{
    enum
    {
        NUM_PARTS = 1024
    };

    hipDeviceProp_t props;
    hipGetDeviceProperties(&props, 0);

    dim3 block(32, 4);
    dim3 grid(16, 16);
    int total_blocks = grid.x * grid.y;

    // allocate partial histogram
    unsigned int *d_part_hist;
    hipMalloc(&d_part_hist, total_blocks * NUM_PARTS * sizeof(unsigned int));

    dim3 block2(128);
    dim3 grid2((3 * NUM_BINS + block.x - 1) / block.x);

    GpuTimer gpu_timer;
    gpu_timer.Start();

    hipLaunchKernelGGL(HIP_KERNEL_NAME(histogram_gmem_atomics<NUM_PARTS, ACTIVE_CHANNELS, NUM_BINS>), grid, block, 0, 0, 
        d_image,
        width,
        height,
        d_part_hist);

    hipLaunchKernelGGL(HIP_KERNEL_NAME(histogram_gmem_accum<NUM_PARTS, ACTIVE_CHANNELS, NUM_BINS>), grid2, block2, 0, 0, 
        d_part_hist,
        total_blocks,
        d_hist);

    gpu_timer.Stop();
    float elapsed_millis = gpu_timer.ElapsedMillis();

    hipFree(d_part_hist);

    return elapsed_millis;
}



// --- from histogram_smem_atomics.h ---
#include "hip/hip_runtime.h"
/******************************************************************************
 * Copyright (c) 2011-2018, NVIDIA CORPORATION.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the NVIDIA CORPORATION nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

    // First-pass histogram kernel (binning into privatized counters)
    template <
        int         NUM_PARTS,
        int         ACTIVE_CHANNELS,
        int         NUM_BINS,
        typename    PixelType>
    void histogram_smem_atomics(
        const PixelType *in,
        int width,
        int height,
        unsigned int *out)
    {
        // global position and size
        int x = _bid_x * BLOCK_DIM_X + _tid_x;
        int y = _bid_y * BLOCK_DIM_Y + _tid_y;
        int nx = BLOCK_DIM_X * GRID_DIM_X;
        int ny = BLOCK_DIM_Y * GRID_DIM_Y;

        // threads in workgroup
        int t = _tid_x + _tid_y * BLOCK_DIM_X; // thread index in workgroup, linear in 0..nt-1
        int nt = BLOCK_DIM_X * BLOCK_DIM_Y; // total threads in workgroup

        // group index in 0..ngroups-1
        int g = _bid_x + _bid_y * GRID_DIM_X;

        // initialize smem
        unsigned int smem[ACTIVE_CHANNELS * NUM_BINS + 3];
        for (int i = t; i < ACTIVE_CHANNELS * NUM_BINS + 3; i += nt)
            smem[i] = 0;

        // process pixels
        // updates our group's partial histogram in smem
        for (int col = x; col < width; col += nx)
        {
            for (int row = y; row < height; row += ny)
            {
                PixelType pixel = in[row * width + col];

                unsigned int bins[ACTIVE_CHANNELS];
                DecodePixel<NUM_BINS>(pixel, bins);

                #pragma unroll
                for (int CHANNEL = 0; CHANNEL < ACTIVE_CHANNELS; ++CHANNEL)
                    atomicAdd(&smem[(NUM_BINS * CHANNEL) + bins[CHANNEL] + CHANNEL], 1);
            }
        }

        // move to our workgroup's slice of output
        out += g * NUM_PARTS;

        // store local output to global
        for (int i = t; i < NUM_BINS; i += nt)
        {
            #pragma unroll
            for (int CHANNEL = 0; CHANNEL < ACTIVE_CHANNELS; ++CHANNEL)
                out[i + NUM_BINS * CHANNEL] = smem[i + NUM_BINS * CHANNEL + CHANNEL];
        }
    }

    // Second pass histogram kernel (accumulation)
    template <
        int         NUM_PARTS,
        int         ACTIVE_CHANNELS,
        int         NUM_BINS>
    void histogram_smem_accum(
        const unsigned int *in,
        int n,
        unsigned int *out)
    {
        int i = _bid_x * BLOCK_DIM_X + _tid_x;
        if (i > ACTIVE_CHANNELS * NUM_BINS) return; // out of range
        unsigned int total = 0;
        for (int j = 0; j < n; j++)
            total += in[i + NUM_PARTS * j];
        out[i] = total;
    }

template <
    int         ACTIVE_CHANNELS,
    int         NUM_BINS,
    typename    PixelType>
double run_smem_atomics(
    PixelType *d_image,
    int width,
    int height,
    unsigned int *d_hist, 
    bool warmup)
{
    enum
    {
        NUM_PARTS = 1024
    };

    hipDeviceProp_t props;
    hipGetDeviceProperties(&props, 0);

    dim3 block(32, 4);
    dim3 grid(16, 16);
    int total_blocks = grid.x * grid.y;

    // allocate partial histogram
    unsigned int *d_part_hist;
    hipMalloc(&d_part_hist, total_blocks * NUM_PARTS * sizeof(unsigned int));

    dim3 block2(128);
    dim3 grid2((ACTIVE_CHANNELS * NUM_BINS + block2.x - 1) / block2.x);

    GpuTimer gpu_timer;
    gpu_timer.Start();

    hipLaunchKernelGGL(HIP_KERNEL_NAME(histogram_smem_atomics<NUM_PARTS, ACTIVE_CHANNELS, NUM_BINS>), dim3(grid), dim3(block), 0, 0, 
        d_image,
        width,
        height,
        d_part_hist);

    hipLaunchKernelGGL(HIP_KERNEL_NAME(histogram_smem_accum<NUM_PARTS, ACTIVE_CHANNELS, NUM_BINS>), dim3(grid2), dim3(block2), 0, 0, 
        d_part_hist,
        total_blocks,
        d_hist);

    gpu_timer.Stop();
    float elapsed_millis = gpu_timer.ElapsedMillis();

    hipFree(d_part_hist);

    return elapsed_millis;
}



// --- from mersenne.h ---
/*
 A C-program for MT19937, with initialization improved 2002/1/26.
 Coded by Takuji Nishimura and Makoto Matsumoto.

 Before using, initialize the state by using init_genrand(seed)
 or init_by_array(init_key, key_length).

 Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 3. The names of its contributors may not be used to endorse or promote
 products derived from this software without specific prior written
 permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 Any feedback is very welcome.
 http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
 email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
 */

#include <stdio.h>

namespace mersenne {

/* Period parameters */
const unsigned int N          = 624;
const unsigned int M          = 397;
const unsigned int MATRIX_A   = 0x9908b0df; /* constant vector a */
const unsigned int UPPER_MASK = 0x80000000; /* most significant w-r bits */
const unsigned int LOWER_MASK = 0x7fffffff; /* least significant r bits */

static unsigned int mt[N];  /* the array for the state vector  */
static int mti = N + 1;     /* mti==N+1 means mt[N] is not initialized */

/* initializes mt[N] with a seed */
void init_genrand(unsigned int s)
{
    mt[0] = s & 0xffffffff;
    for (mti = 1; mti < N; mti++)
    {
        mt[mti] = (1812433253 * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);

        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for mtiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */

        mt[mti] &= 0xffffffff;
        /* for >32 bit machines */
    }
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void init_by_array(unsigned int init_key[], int key_length)
{
    int i, j, k;
    init_genrand(19650218);
    i = 1;
    j = 0;
    k = (N > key_length ? N : key_length);
    for (; k; k--)
    {
        mt[i] = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 30)) * 1664525))
            + init_key[j] + j;  /* non linear */
        mt[i] &= 0xffffffff;    /* for WORDSIZE > 32 machines */
        i++;
        j++;
        if (i >= N)
        {
            mt[0] = mt[N - 1];
            i = 1;
        }
        if (j >= key_length) j = 0;
    }
    for (k = N - 1; k; k--)
    {
        mt[i] = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 30)) * 1566083941)) - i; /* non linear */
        mt[i] &= 0xffffffff; /* for WORDSIZE > 32 machines */
        i++;
        if (i >= N)
        {
            mt[0] = mt[N - 1];
            i = 1;
        }
    }

    mt[0] = 0x80000000; /* MSB is 1; assuring non-zero initial array */
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned int genrand_int32(void)
{
    unsigned int y;
    static unsigned int mag01[2] = { 0x0, MATRIX_A };

    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N)
    { /* generate N words at one time */
        int kk;

        if (mti == N + 1) /* if init_genrand() has not been called, */
        init_genrand(5489); /* a defat initial seed is used */

        for (kk = 0; kk < N - M; kk++)
        {
            y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
            mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        for (; kk < N - 1; kk++)
        {
            y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
            mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
        mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1];

        mti = 0;
    }

    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680;
    y ^= (y << 15) & 0xefc60000;
    y ^= (y >> 18);

    return y;
}

} // namespace mersenne


// --- from test_util.h ---
/******************************************************************************
 * Copyright (c) 2011, Duane Merrill.  All rights reserved.
 * Copyright (c) 2011-2018, NVIDIA CORPORATION.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the NVIDIA CORPORATION nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #undef small            // Windows is terrible for polluting macro namespace
#else
    #include <sys/resource.h>
#endif

#include <hip/hip_runtime.h>

#include <stdio.h>
#include <math.h>
#include <float.h>

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <limits>


/******************************************************************************
 * Assertion macros
 ******************************************************************************/

/**
 * Assert equals
 */
#define AssertEquals(a, b) if ((a) != (b)) { std::cerr << "\n(" << __FILE__ << ": " << __LINE__ << ")\n"; exit(1);}

/******************************************************************************
 * Command-line parsing functionality
 ******************************************************************************/

/**
 * Utility for parsing command line arguments
 */
struct CommandLineArgs
{

    std::vector<std::string>    keys;
    std::vector<std::string>    values;
    std::vector<std::string>    args;
    hipDeviceProp_t              deviceProp;
    float                       device_giga_bandwidth;
    size_t                      device_free_physmem;
    size_t                      device_total_physmem;

    /**
     * Constructor
     */
    CommandLineArgs(int argc, char **argv) :
        keys(10),
        values(10)
    {
        using namespace std;

        // Initialize mersenne generator
        unsigned int mersenne_init[4]=  {0x123, 0x234, 0x345, 0x456};
        mersenne::init_by_array(mersenne_init, 4);

        for (int i = 1; i < argc; i++)
        {
            string arg = argv[i];

            if ((arg[0] != '-') || (arg[1] != '-'))
            {
                args.push_back(arg);
                continue;
            }

            string::size_type pos;
            string key, val;
            if ((pos = arg.find('=')) == string::npos) {
                key = string(arg, 2, arg.length() - 2);
                val = "";
            } else {
                key = string(arg, 2, pos - 2);
                val = string(arg, pos + 1, arg.length() - 1);
            }

            keys.push_back(key);
            values.push_back(val);
        }
    }

    /**
     * Checks whether a flag "--<flag>" is present in the commandline
     */
    bool CheckCmdLineFlag(const char* arg_name)
    {
        using namespace std;

        for (int i = 0; i < int(keys.size()); ++i)
        {
            if (keys[i] == string(arg_name))
                return true;
        }
        return false;
    }

    /**
     * Returns number of naked (non-flag and non-key-value) commandline parameters
     */
    template <typename T>
    int NumNakedArgs()
    {
        return args.size();
    }

    /**
     * Returns the commandline parameter for a given index (not including flags)
     */
    template <typename T>
    void GetCmdLineArgument(int index, T &val)
    {
        using namespace std;
        if (index < args.size()) {
            istringstream str_stream(args[index]);
            str_stream >> val;
        }
    }

    /**
     * Returns the value specified for a given commandline parameter --<flag>=<value>
     */
    template <typename T>
    void GetCmdLineArgument(const char *arg_name, T &val)
    {
        using namespace std;

        for (int i = 0; i < int(keys.size()); ++i)
        {
            if (keys[i] == string(arg_name))
            {
                istringstream str_stream(values[i]);
                str_stream >> val;
            }
        }
    }

    /**
     * Returns the values specified for a given commandline parameter --<flag>=<value>,<value>*
     */
    template <typename T>
    void GetCmdLineArguments(const char *arg_name, std::vector<T> &vals)
    {
        using namespace std;

        if (CheckCmdLineFlag(arg_name))
        {
            // Clear any default values
            vals.clear();

            // Recover from multi-value string
            for (int i = 0; i < keys.size(); ++i)
            {
                if (keys[i] == string(arg_name))
                {
                    string val_string(values[i]);
                    istringstream str_stream(val_string);
                    string::size_type old_pos = 0;
                    string::size_type new_pos = 0;

                    // Iterate comma-separated values
                    T val;
                    while ((new_pos = val_string.find(',', old_pos)) != string::npos)
                    {
                        if (new_pos != old_pos)
                        {
                            str_stream.width(new_pos - old_pos);
                            str_stream >> val;
                            vals.push_back(val);
                        }

                        // skip over comma
                        str_stream.ignore(1);
                        old_pos = new_pos + 1;
                    }

                    // Read last value
                    str_stream >> val;
                    vals.push_back(val);
                }
            }
        }
    }

    /**
     * The number of pairs parsed
     */
    int ParsedArgc()
    {
        return (int) keys.size();
    }

    /**
     * Initialize device
     */
    /*
    hipError_t DeviceInit(int dev = -1)
    {
        hipError_t error = hipSuccess;

        do
        {
            int deviceCount;
            error = hipGetDeviceCount(&deviceCount);
            if (error) break;

            if (deviceCount == 0) {
                fprintf(stderr, "No devices supporting CUDA.\n");
                exit(1);
            }
            if (dev < 0)
            {
                GetCmdLineArgument("device", dev);
            }
            if ((dev > deviceCount - 1) || (dev < 0))
            {
                dev = 0;
            }

            error = hipSetDevice(dev);
            if (error) break;

            hipMemGetInfo(&device_free_physmem, &device_total_physmem);

            error = hipGetDeviceProperties(&deviceProp, dev);
            if (error) break;

            if (deviceProp.major < 1) {
                fprintf(stderr, "Device does not support CUDA.\n");
                exit(1);
            }

            device_giga_bandwidth = float(deviceProp.memoryBusWidth) * deviceProp.memoryClockRate * 2 / 8 / 1000 / 1000;

            if (!CheckCmdLineFlag("quiet"))
            {
                printf(
                        "Using device %d: %s (%d SMs, "
                        "%lld free / %lld total MB physmem, "
                        "%.3f GB/s @ %d kHz mem clock)\n",
                    dev,
                    deviceProp.name,
                    deviceProp.multiProcessorCount,
                    (unsigned long long) device_free_physmem / 1024 / 1024,
                    (unsigned long long) device_total_physmem / 1024 / 1024,
                    device_giga_bandwidth,
                    deviceProp.memoryClockRate);
                fflush(stdout);
            }

        } while (0);

        return error;
    }
    */
};

/******************************************************************************
 * Random bits generator
 ******************************************************************************/

int g_num_rand_samples = 0;

template <typename T>
bool IsNaN(T val) { return false; }

template<>
 bool IsNaN<float>(float val)
{
    volatile unsigned int bits = reinterpret_cast<unsigned int &>(val);

    return (((bits >= 0x7F800001) && (bits <= 0x7FFFFFFF)) || 
        ((bits >= 0xFF800001) && (bits <= 0xFFFFFFFF)));
}

/*
template<>
 bool IsNaN<float1>(float1 val)
{
    return (IsNaN(val.x));
}
*/

template<>
 bool IsNaN<float2>(float2 val)
{
    return (IsNaN(val.y) || IsNaN(val.x));
}

template<>
 bool IsNaN<float3>(float3 val)
{
    return (IsNaN(val.z) || IsNaN(val.y) || IsNaN(val.x));
}

template<>
 bool IsNaN<float4>(float4 val)
{
    return (IsNaN(val.y) || IsNaN(val.x) || IsNaN(val.w) || IsNaN(val.z));
}

template<>
 bool IsNaN<double>(double val)
{
    volatile unsigned long long bits = *reinterpret_cast<unsigned long long *>(&val);

    return (((bits >= 0x7FF0000000000001) && (bits <= 0x7FFFFFFFFFFFFFFF)) || 
        ((bits >= 0xFFF0000000000001) && (bits <= 0xFFFFFFFFFFFFFFFF)));
}

/*
template<>
 bool IsNaN<double1>(double1 val)
{
    return (IsNaN(val.x));
}
*/

template<>
 bool IsNaN<double2>(double2 val)
{
    return (IsNaN(val.y) || IsNaN(val.x));
}

template<>
 bool IsNaN<double3>(double3 val)
{
    return (IsNaN(val.z) || IsNaN(val.y) || IsNaN(val.x));
}

template<>
 bool IsNaN<double4>(double4 val)
{
    return (IsNaN(val.y) || IsNaN(val.x) || IsNaN(val.w) || IsNaN(val.z));
}

/******************************************************************************
 * Console printing utilities
 ******************************************************************************/

/**
 * Helper for casting character types to integers for cout printing
 */
template <typename T>
T CoutCast(T val) { return val; }

int CoutCast(char val) { return val; }

int CoutCast(unsigned char val) { return val; }

int CoutCast(signed char val) { return val; }

/******************************************************************************
 * Test value initialization utilities
 ******************************************************************************/

/**
 * Test problem generation options
 */
enum GenMode
{
    UNIFORM,            // Assign to '2', regardless of integer seed
    INTEGER_SEED,       // Assign to integer seed
};

/**
 * Initialize value
 */
template <typename T>
inline void InitValue(GenMode gen_mode, T &value, int index = 0)
{
    switch (gen_mode)
    {
     case UNIFORM:
        value = 2;
        break;
    case INTEGER_SEED:
    default:
         value = (T) index;
        break;
    }
}

/**
 * Initialize value (bool)
 */
inline void InitValue(GenMode gen_mode, bool &value, int index = 0)
{
    switch (gen_mode)
    {
     case UNIFORM:
        value = true;
        break;
    case INTEGER_SEED:
    default:
        value = (index > 0);
        break;
    }
}

/**
 * Generates random keys.
 *
 * We always take the second-order byte from rand() because the higher-order
 * bits returned by rand() are commonly considered more uniformly distributed
 * than the lower-order bits.
 *
 * We can decrease the entropy level of keys by adopting the technique
 * of Thearling and Smith in which keys are computed from the bitwise AND of
 * multiple random samples:
 *
 * entropy_reduction    | Effectively-unique bits per key
 * -----------------------------------------------------
 * -1                   | 0
 * 0                    | 32
 * 1                    | 25.95 (81%)
 * 2                    | 17.41 (54%)
 * 3                    | 10.78 (34%)
 * 4                    | 6.42 (20%)
 * ...                  | ...
 *
 */
template <typename K>
void RandomBits(
    K &key,
    int entropy_reduction = 0,
    int begin_bit = 0,
    int end_bit = sizeof(K) * 8)
{
    const int NUM_BYTES = sizeof(K);
    const int WORD_BYTES = sizeof(unsigned int);
    const int NUM_WORDS = (NUM_BYTES + WORD_BYTES - 1) / WORD_BYTES;

    unsigned int word_buff[NUM_WORDS];

    if (entropy_reduction == -1)
    {
        memset((void *) &key, 0, sizeof(key));
        return;
    }

    if (end_bit < 0)
        end_bit = sizeof(K) * 8;

    while (true) 
    {
        // Generate random word_buff
        for (int j = 0; j < NUM_WORDS; j++)
        {
            int current_bit = j * WORD_BYTES * 8;

            unsigned int word = 0xffffffff;
            word &= 0xffffffff << std::max(0, begin_bit - current_bit);
            word &= 0xffffffff >> std::max(0, (current_bit + (WORD_BYTES * 8)) - end_bit);

            for (int i = 0; i <= entropy_reduction; i++)
            {
                // Grab some of the higher bits from rand (better entropy, supposedly)
                word &= mersenne::genrand_int32();
                g_num_rand_samples++;                
            }

            word_buff[j] = word;
        }

        memcpy(&key, word_buff, sizeof(K));

        K copy = key;
        if (!IsNaN(copy))
            break;          // avoids NaNs when generating random floating point numbers
    }
}

/******************************************************************************
 * Helper routines for list comparison and display
 ******************************************************************************/

/**
 * Compares the equivalence of two arrays
 */
template <typename S, typename T, typename OffsetT>
int CompareResults(T* computed, S* reference, OffsetT len, bool verbose = true)
{
    for (OffsetT i = 0; i < len; i++)
    {
        if (computed[i] != reference[i])
        {
            if (verbose) std::cout << "INCORRECT: [" << i << "]: "
                << CoutCast(computed[i]) << " != "
                << CoutCast(reference[i]);
            return 1;
        }
    }
    return 0;
}

/**
 * Compares the equivalence of two arrays
 */
template <typename OffsetT>
int CompareResults(float* computed, float* reference, OffsetT len, bool verbose = true)
{
    for (OffsetT i = 0; i < len; i++)
    {
        if (computed[i] != reference[i])
        {
            float difference = std::abs(computed[i]-reference[i]);
            float fraction = difference / std::abs(reference[i]);

            if (fraction > 0.0001)
            {
                if (verbose) std::cout << "INCORRECT: [" << i << "]: "
                    << "(computed) " << CoutCast(computed[i]) << " != "
                    << CoutCast(reference[i]) << " (difference:" << difference << ", fraction: " << fraction << ")";
                return 1;
            }
        }
    }
    return 0;
}

/**
 * Compares the equivalence of two arrays
 */
template <typename OffsetT>
int CompareResults(double* computed, double* reference, OffsetT len, bool verbose = true)
{
    for (OffsetT i = 0; i < len; i++)
    {
        if (computed[i] != reference[i])
        {
            double difference = std::abs(computed[i]-reference[i]);
            double fraction = difference / std::abs(reference[i]);

            if (fraction > 0.0001)
            {
                if (verbose) std::cout << "INCORRECT: [" << i << "]: "
                    << CoutCast(computed[i]) << " != "
                    << CoutCast(reference[i]) << " (difference:" << difference << ", fraction: " << fraction << ")";
                return 1;
            }
        }
    }
    return 0;
}

/**
 * Verify the contents of a device array match those
 * of a host array
 */
template <typename S, typename T>
int CompareDeviceResults(
    S *h_reference,
    T *d_data,
    size_t num_items,
    bool verbose = true,
    bool display_data = false)
{
    // Allocate array on host
    T *h_data = (T*) malloc(num_items * sizeof(T));

    // Copy data back
    hipMemcpy(h_data, d_data, sizeof(T) * num_items, hipMemcpyDeviceToHost);

    // Display data
    if (display_data)
    {
        printf("Reference:\n");
        for (int i = 0; i < int(num_items); i++)
        {
            std::cout << CoutCast(h_reference[i]) << ", ";
        }
        printf("\n\nComputed:\n");
        for (int i = 0; i < int(num_items); i++)
        {
            std::cout << CoutCast(h_data[i]) << ", ";
        }
        printf("\n\n");
    }

    // Check
    int retval = CompareResults(h_data, h_reference, num_items, verbose);

    // Cleanup
    if (h_data) free(h_data);

    return retval;
}

/******************************************************************************
 * Timing
 ******************************************************************************/

struct CpuTimer
{
#if defined(_WIN32) || defined(_WIN64)

    LARGE_INTEGER ll_freq;
    LARGE_INTEGER ll_start;
    LARGE_INTEGER ll_stop;

    CpuTimer()
    {
        QueryPerformanceFrequency(&ll_freq);
    }

    void Start()
    {
        QueryPerformanceCounter(&ll_start);
    }

    void Stop()
    {
        QueryPerformanceCounter(&ll_stop);
    }

    float ElapsedMillis()
    {
        double start = double(ll_start.QuadPart) / double(ll_freq.QuadPart);
        double stop  = double(ll_stop.QuadPart) / double(ll_freq.QuadPart);

        return float((stop - start) * 1000);
    }

#else

    rusage start;
    rusage stop;

    void Start()
    {
        getrusage(RUSAGE_SELF, &start);
    }

    void Stop()
    {
        getrusage(RUSAGE_SELF, &stop);
    }

    float ElapsedMillis()
    {
        float sec = stop.ru_utime.tv_sec - start.ru_utime.tv_sec;
        float usec = stop.ru_utime.tv_usec - start.ru_utime.tv_usec;

        return (sec * 1000) + (usec / 1000);
    }

#endif
};

struct GpuTimer
{
    hipEvent_t start;
    hipEvent_t stop;

    GpuTimer()
    {
        hipEventCreate(&start);
        hipEventCreate(&stop);
    }

    ~GpuTimer()
    {
        hipEventDestroy(start);
        hipEventDestroy(stop);
    }

    void Start()
    {
        hipEventRecord(start, 0);
    }

    void Stop()
    {
        hipEventRecord(stop, 0);
    }

    float ElapsedMillis()
    {
        float elapsed;
        hipEventSynchronize(stop);
        hipEventElapsedTime(&elapsed, start, stop);
        return elapsed;
    }
};
