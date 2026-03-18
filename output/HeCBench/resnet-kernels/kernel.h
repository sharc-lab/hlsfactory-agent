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
#ifndef BLOCK_DIM_Z
#define BLOCK_DIM_Z 1
#endif
#ifndef GRID_DIM_Z
#define GRID_DIM_Z 1
#endif

// --- from Kernel128_one.cu ---

const char inputName128one[] = "data/input_one_14_1024.bin";
const char weightName128one[] = "data/weight_one_1024.bin";
const char bnBias_myKernel_Name128one[] = "data/bnBias_myKernel_one_1024.bin";
const char bnScale_myKernel_Name128one[] = "data/bnScale_myKernel_one_1024.bin";






// --- from Kernel128_winograd.cu ---

const char inputName128[] = "data/input_14_1_128.bin";
const char biasName128[] = "data/bias_128.bin";
const char weight_winograd_Name128[] = "data/weight_winograd_128_128.bin";
const char bnBias_winograd_Name128[] = "data/bnBias_winograd_128.bin";
const char bnScale_winograd_Name128[] = "data/bnScale_winograd_128.bin";

#define d(input, i, j, Inz) ( input[Inz + i*768 + (j<<7)] )






// --- from Kernel256_one.cu ---

const char inputName256one[] = "data/input_one_14_1024.bin";
const char weightName256one[] = "data/weight_one_1024.bin";
const char bnBias_myKernel_Name256one[] = "data/bnBias_myKernel_one_1024.bin";
const char bnScale_myKernel_Name256one[] = "data/bnScale_myKernel_one_1024.bin";






// --- from Kernel256_winograd.cu ---

const char inputName256[] = "data/input_14_1_256.bin";
const char weight_winograd_Name256[] = "data/weight_winograd_256_256.bin";
const char bnBias_winograd_Name256[] = "data/bnBias_winograd_256.bin";
const char bnScale_winograd_Name256[] = "data/bnScale_winograd_256.bin";

#define d(input, i, j, Inz) ( input[Inz + i*768 + (j<<7)] )






// --- from main.cu ---



// --- from util.cu ---





// --- from Kernel128_one.h ---
#ifndef __KERNEL128_ONE_H__
#define __KERNEL128_ONE_H__

void kernel_128_1_in(double&, double&);
void kernel_128_1_out(double&, double&);

#endif


// --- from Kernel128_winograd.h ---
#ifndef __KERNEL128_WINOGRAD_H__
#define __KERNEL128_WINOGRAD_H__

void kernel_128(double&, double&);

#endif


// --- from Kernel256_one.h ---
#ifndef __KERNEL256_ONE_H__
#define __KERNEL256_ONE_H__

void kernel_256_1_in(double&, double&);
void kernel_256_1_out(double&, double&);

#endif


// --- from Kernel256_winograd.h ---
#ifndef __KERNEL256_WINOGRAD_H__
#define __KERNEL256_WINOGRAD_H__

void kernel_256(double&, double&);

#endif


// --- from util.h ---
#ifndef __UTIL_H__
#define __UTIL_H__

#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>

float* get_parameter(const char* filename, int size);

float* transpose(float* weight, int h, int w);

void output_checker(float* A, float* B, int len, int channel, int shift);

#endif
