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

// --- from heat.cu ---
/*
** PROGRAM: heat equation solve
**
** PURPOSE: This program will explore use of an explicit
**          finite difference method to solve the heat
**          equation under a method of manufactured solution (MMS)
**          scheme. The solution has been set to be a simple 
**          function based on exponentials and trig functions.
**
**          A finite difference scheme is used on a 1000x1000 cube.
**          A total of 0.5 units of time are simulated.
**
**          The MMS solution has been adapted from
**          G.W. Recktenwald (2011). Finite difference approximations
**          to the Heat Equation. Portland State University.
**
**
** USAGE:   Run with two arguments:
**          First is the number of cells.
**          Second is the number of timesteps.
**
**          For example, with 100x100 cells and 10 steps:
**
**          ./heat 100 10
**
**
** HISTORY: Written by Tom Deakin, Oct 2018
**          Ported to SYCL by Tom Deakin, Nov 2019
**          Ported to OpenCL by Tom Deakin, Jan 2020
**
*/

#include <iostream>
#include <chrono>
#include <cmath>
#include <fstream>

#include <hip/hip_runtime.h>

// Key constants used in this program
#define LINE "--------------------" // A line for fancy output

void initial_value(const unsigned int n, const double dx, const double length, double * u);
void zero(const unsigned int n, double * u);
void solve(const unsigned int n, const double alpha, const double dx, const double dt, const double r, const double r2,
		double *  u, double *  u_tmp);
double solution(const double t, const double x, const double y, const double alpha, const double length);

// Sets the mesh to an initial value, determined by the MMS scheme

// Zero the array u

// Compute the next timestep, given the current timestep
// Loop over the nxn grid

// True answer given by the manufactured solution

// Computes the L2-norm of the computed grid and the MMS known solution
// The known solution is the same as the boundary function.
