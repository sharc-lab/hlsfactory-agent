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
/**
 * main.cpp: This file is part of the gpumembench micro-benchmark suite.
 *
 * Contact: Elias Konstantinidis <ekondis@gmail.com>
 **/

#include <stdio.h>
#include <stdlib.h>
#include <chrono>

#define VECTOR_SIZE 1024

// Initialize vector data
template <class T>

template <>

template <>

template <>

// Sum up vector data
template <class T>

template <>

template <>

template <>

template <class T>




int constant_data[VECTOR_SIZE];

template <class T>

template<typename T>


