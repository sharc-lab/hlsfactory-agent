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

// --- from kernels.cu ---
#include <chrono>

template <typename CmplxType>

template <typename CmplxType>


// --- from main.cu ---
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <hip/hip_runtime.h>



template <class T,class Thalf>



// --- from degrid.h ---
#ifndef __DEGRID_H
#define __DEGRID_H

// NPOINTS is a multiple of 32
#define NPOINTS  40000
#define GCF_DIM  256 
#define IMG_SIZE 8192
#define GCF_GRID 8
#define REPEAT   100

// define PRECISION in Makefile
#define PASTER(x) x ## 2
#define EVALUATOR(x) PASTER(x)
#define PRECISION2 EVALUATOR(PRECISION)

#endif
