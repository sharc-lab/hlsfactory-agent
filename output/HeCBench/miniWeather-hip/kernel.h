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

// --- from main.cu ---
//
// Original author: Matt Norman <normanmr@ornl.gov>  , Oak Ridge National Laboratory
//
// miniWeather simulates dry, stratified, compressible, non-hydrostatic fluid flows
//

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <chrono>
#include <mpi.h>
#include <hip/hip_runtime.h>
#include "check_output.h"

const double pi        = 3.14159265358979323846264338327;   //Pi
const double grav      = 9.8;                               //Gravitational acceleration (m / s^2)
const double cp        = 1004.;                             //Specific heat of dry air at constant pressure
const double cv        = 717.;                              //Specific heat of dry air at constant volume
const double rd        = 287.;                              //Dry air constant for equation of state (P=rho*rd*T)
const double p0        = 1.e5;                              //Standard pressure at the surface in Pascals
const double C0        = 27.5629410929725921310572974482;   //Constant to translate potential temperature into pressure (P=C0*(rho*theta)**gamma)
const double gamm      = 1.40027894002789400278940027894;   //gamma=cp/Rd , have to call this gamm because "gamma" is taken (I hate C so much)
//Define domain and stability-related constants
const double xlen      = 2.e4;    //Length of the domain in the x-direction (meters)
const double zlen      = 1.e4;    //Length of the domain in the z-direction (meters)
const double hv_beta   = 0.25;     //How strong to diffuse the solution: hv_beta \in [0:1]
const double cfl       = 1.50;    //"Courant, Friedrichs, Lewy" number (for numerical stability)
const double max_speed = 450;        //Assumed maximum wave speed during the simulation (speed of sound + speed of wind) (meter / sec)
const int hs        = 2;          //"Halo" size: number of cells beyond the MPI tasks's domain needed for a full "stencil" of information for reconstruction
const int sten_size = 4;          //Size of the stencil used for interpolation

//Parameters for indexing and flags
const int NUM_VARS = 4;           //Number of fluid state variables
const int ID_DENS  = 0;           //index for density ("rho")
const int ID_UMOM  = 1;           //index for momentum in the x-direction ("rho * u")
const int ID_WMOM  = 2;           //index for momentum in the z-direction ("rho * w")
const int ID_RHOT  = 3;           //index for density * potential temperature ("rho * theta")
const int DIR_X = 1;              //Integer constant to express that this operation is in the x-direction
const int DIR_Z = 2;              //Integer constant to express that this operation is in the z-direction
const int DATA_SPEC_COLLISION       = 1;
const int DATA_SPEC_THERMAL         = 2;
const int DATA_SPEC_MOUNTAIN        = 3;
const int DATA_SPEC_TURBULENCE      = 4;
const int DATA_SPEC_DENSITY_CURRENT = 5;
const int DATA_SPEC_INJECTION       = 6;

const int nqpoints = 3;
double qpoints [] = { 0.112701665379258311482073460022E0 , 0.500000000000000000000000000000E0 , 0.887298334620741688517926539980E0 };
double qweights[] = { 0.277777777777777777777777777779E0 , 0.444444444444444444444444444444E0 , 0.277777777777777777777777777779E0 };

///////////////////////////////////////////////////////////////////////////////////////
// Variables that are initialized but remain static over the coure of the simulation
///////////////////////////////////////////////////////////////////////////////////////
double sim_time;              //total simulation time in seconds
double dt;                    //Model time step (seconds)
int    nx, nz;                //Number of local grid cells in the x- and z- dimensions for this MPI task
double dx, dz;                //Grid space length in x- and z-dimension (meters)
int    nx_glob, nz_glob;      //Number of total grid cells in the x- and z- dimensions
int    i_beg, k_beg;          //beginning index in the x- and z-directions for this MPI task
int    nranks, myrank;        //Number of MPI ranks and my rank id
int    left_rank, right_rank; //MPI Rank IDs that exist to my left and right in the global domain
int    masterproc;            //Am I the master process (rank == 0)?
double data_spec_int;         //Which data initialization to use
double *hy_dens_cell;         //hydrostatic density (vert cell avgs).   Dimensions: (1-hs:nz+hs)
double *hy_dens_theta_cell;   //hydrostatic rho*t (vert cell avgs).     Dimensions: (1-hs:nz+hs)
double *hy_dens_int;          //hydrostatic density (vert cell interf). Dimensions: (1:nz+1)
double *hy_dens_theta_int;    //hydrostatic rho*t (vert cell interf).   Dimensions: (1:nz+1)
double *hy_pressure_int;      //hydrostatic press (vert cell interf).   Dimensions: (1:nz+1)

///////////////////////////////////////////////////////////////////////////////////////
// Variables that are dynamics over the course of the simulation
///////////////////////////////////////////////////////////////////////////////////////
double etime;                 //Elapsed model time
double output_counter;        //Helps determine when it's time to do output
//Runtime variable arrays
double *state;                //Fluid state.             Dimensions: (1-hs:nx+hs,1-hs:nz+hs,NUM_VARS)
double *state_tmp;            //Fluid state.             Dimensions: (1-hs:nx+hs,1-hs:nz+hs,NUM_VARS)
double *flux;                 //Cell interface fluxes.   Dimensions: (nx+1,nz+1,NUM_VARS)
double *tend;                 //Fluid state tendencies.  Dimensions: (nx,nz,NUM_VARS)
double *sendbuf_l;            //Buffer to send data to the left MPI rank
double *sendbuf_r;            //Buffer to send data to the right MPI rank
double *recvbuf_l;            //Buffer to receive data from the left MPI rank
double *recvbuf_r;            //Buffer to receive data from the right MPI rank
int    num_out = 0;           //The number of outputs performed so far
int    direction_switch = 1;
double mass0, te0;            //Initial domain totals for mass and total energy
double mass , te ;            //Domain totals for mass and total energy


//Establish hydrstatic balance using constant potential temperature (thermally neutral atmosphere)
//z is the input coordinate
//r and t are the output background hydrostatic density and potential temperature

//Establish hydrstatic balance using constant Brunt-Vaisala frequency
//z is the input coordinate
//bv_freq0 is the constant Brunt-Vaisala frequency
//r and t are the output background hydrostatic density and potential temperature

//Sample from an ellipse of a specified center, radius, and amplitude at a specified location
//x and z are input coordinates
//amp,x0,z0,xrad,zrad are input amplitude, center, and radius of the ellipse

//This test case is initially balanced but injects fast, cold air from the left boundary near the model top
//x and z are input coordinates at which to sample
//r,u,w,t are output density, u-wind, w-wind, and potential temperature at that location
//hr and ht are output background hydrostatic density and potential temperature at that location

//Initialize a density current (falling cold thermal that propagates along the model bottom)
//x and z are input coordinates at which to sample
//r,u,w,t are output density, u-wind, w-wind, and potential temperature at that location
//hr and ht are output background hydrostatic density and potential temperature at that location

//x and z are input coordinates at which to sample
//r,u,w,t are output density, u-wind, w-wind, and potential temperature at that location
//hr and ht are output background hydrostatic density and potential temperature at that location

//x and z are input coordinates at which to sample
//r,u,w,t are output density, u-wind, w-wind, and potential temperature at that location
//hr and ht are output background hydrostatic density and potential temperature at that location

//Rising thermal
//x and z are input coordinates at which to sample
//r,u,w,t are output density, u-wind, w-wind, and potential temperature at that location
//hr and ht are output background hydrostatic density and potential temperature at that location

//Colliding thermals
//x and z are input coordinates at which to sample
//r,u,w,t are output density, u-wind, w-wind, and potential temperature at that location
//hr and ht are output background hydrostatic density and potential temperature at that location

//Compute the time tendencies of the fluid state using forcing in the x-direction
//Since the halos are set in a separate routine, this will not require MPI
//First, compute the flux vector at each cell interface in the x-direction (including hyperviscosity)
//Then, compute the tendencies using those fluxes

//Compute the time tendencies of the fluid state using forcing in the z-direction
//Since the halos are set in a separate routine, this will not require MPI
//First, compute the flux vector at each cell interface in the z-direction (including hyperviscosity)
//Then, compute the tendencies using those fluxes

//Set this MPI task's halo values in the x-direction. This routine will require MPI

//Set this MPI task's halo values in the z-direction. This does not require MPI because there is no MPI
//decomposition in the vertical direction



//Compute reduced quantities for error checking without resorting to the "ncdiff" tool
//#pragma omp target teams distribute parallel for collapse(2) reduction(+:mass,te)

//Perform a single semi-discretized step in time with the form:
//state_out = state_init + dt * rhs(state_forcing)
//Meaning the step starts from state_init, computes the rhs using state_forcing, and stores the result in state_out
void semi_discrete_step(
    const int hs,
    const int nx,
    const int nz,
    const int k_beg,
    const int i_beg,
    const double dx,
    const double dz,
    const double dt,
    int dir,
    const int data_spec_int,
    double *d_state_init ,
    double *d_state_forcing ,
    double *d_state_out,
    double *d_flux ,
    double *d_tend,
    double *d_hy_dens_cell ,
    double *d_hy_dens_theta_cell ,
    double *d_hy_dens_int ,
    double *d_hy_dens_theta_int ,
    double *d_hy_pressure_int ,
    double *d_sendbuf_l ,
    double *d_sendbuf_r ,
    double *d_recvbuf_l ,
    double *d_recvbuf_r)
{

  //Apply the tendencies to the fluid state
  dim3 tend_gws ((nx+15)/16, (nz+15)/16, NUM_VARS);
  dim3 tend_lws (16, 16, 1);

    nx, nz, hs, dt);
}

//Performs a single dimensionally split time step using a simple low-storate three-stage Runge-Kutta time integrator
//The dimensional splitting is a second-order-accurate alternating Strang splitting in which the
//order of directions is alternated each time step.
//The Runge-Kutta method used here is defined as follows:
// q*     = q[n] + dt/3 * rhs(q[n])
// q**    = q[n] + dt/2 * rhs(q*  )
// q[n+1] = q[n] + dt/1 * rhs(q** )

// THE MAIN PROGRAM STARTS HERE


// --- from kernels.h ---
/*
 * Copyright 2010 by NVIDIA Corporation.  All rights reserved.  All
 * information contained herein is proprietary and confidential to NVIDIA
 * Corporation.  Any use, reproduction, or disclosure without the written
 * permission of NVIDIA Corporation is prohibited.

  Double-precision floating point atomic add
 */

inline
double atomic_add(double *address, double val)
{
  // Doing it all as longlongs cuts one __longlong_as_double from the inner loop
  unsigned long long *ptr = (unsigned long long *)address;
  unsigned long long old, newdbl, ret = *ptr;
  do {
    old = ret;
    newdbl = __double_as_longlong(__longlong_as_double(old)+val);
  } while((ret = atomicCAS(ptr, old, newdbl)) != old);
  return __longlong_as_double(ret);
}

void compute_flux_x (const double * state,
                           double * flux,
                     const double * hy_dens_cell,
                     const double * hy_dens_theta_cell,
                     const double hv_coef,
                     const int nx,
                     const int nz,
                     const int hs)
{
  int k = _bid_y * BLOCK_DIM_Y + _tid_y;
  int i = _bid_x * BLOCK_DIM_X + _tid_x;
  double stencil[4], d3_vals[NUM_VARS], vals[NUM_VARS];

  if (i < nx+1 && k < nz) {
    //Use fourth-order interpolation from four cell averages to compute the value at the interface in question
    for (int ll=0; ll<NUM_VARS; ll++) {
      for (int s=0; s < sten_size; s++) {
        int inds = ll*(nz+2*hs)*(nx+2*hs) + (k+hs)*(nx+2*hs) + i+s;
        stencil[s] = state[inds];
      }
      //Fourth-order-accurate interpolation of the state
      vals[ll] = -stencil[0]/12 + 7*stencil[1]/12 + 7*stencil[2]/12 - stencil[3]/12;
      //First-order-accurate interpolation of the third spatial derivative of the state (for artificial viscosity)
      d3_vals[ll] = -stencil[0] + 3*stencil[1] - 3*stencil[2] + stencil[3];
    }

    //Compute density, u-wind, w-wind, potential temperature, and pressure (r,u,w,t,p respectively)
    double r = vals[ID_DENS] + hy_dens_cell[k+hs];
    double u = vals[ID_UMOM] / r;
    double w = vals[ID_WMOM] / r;
    double t = ( vals[ID_RHOT] + hy_dens_theta_cell[k+hs] ) / r;
    double p = C0*pow((r*t),gamm);

    //Compute the flux vector
    flux[ID_DENS*(nz+1)*(nx+1) + k*(nx+1) + i] = r*u     - hv_coef*d3_vals[ID_DENS];
    flux[ID_UMOM*(nz+1)*(nx+1) + k*(nx+1) + i] = r*u*u+p - hv_coef*d3_vals[ID_UMOM];
    flux[ID_WMOM*(nz+1)*(nx+1) + k*(nx+1) + i] = r*u*w   - hv_coef*d3_vals[ID_WMOM];
    flux[ID_RHOT*(nz+1)*(nx+1) + k*(nx+1) + i] = r*u*t   - hv_coef*d3_vals[ID_RHOT];
  }
}

void compute_tend_x (const double * flux,
                           double * tend,
                     const int nx,
                     const int nz,
                     const int dx )
{
  int ll = _bid_z * BLOCK_DIM_Z + _tid_z;
  int k = _bid_y * BLOCK_DIM_Y + _tid_y;
  int i = _bid_x * BLOCK_DIM_X + _tid_x;
  if (i < nx && k < nz) {
    int indt  = ll* nz   * nx    + k* nx    + i  ;
    int indf1 = ll*(nz+1)*(nx+1) + k*(nx+1) + i  ;
    int indf2 = ll*(nz+1)*(nx+1) + k*(nx+1) + i+1;
    tend[indt] = -( flux[indf2] - flux[indf1] ) / dx;
  }
}

void compute_flux_z (const double * state,
                           double * flux,
                           double * hy_dens_int,
                           double * hy_pressure_int,
                           double * hy_dens_theta_int,
                     const double hv_coef,
                     const int nx,
                     const int nz,
                     const int hs)
{
  int k = _bid_y * BLOCK_DIM_Y + _tid_y;
  int i = _bid_x * BLOCK_DIM_X + _tid_x;
  double stencil[4], d3_vals[NUM_VARS], vals[NUM_VARS];

  if (i < nx && k < nz+1) {
    //Use fourth-order interpolation from four cell averages to compute the value at the interface in question
    for (int ll=0; ll<NUM_VARS; ll++) {
      for (int s=0; s<sten_size; s++) {
        int inds = ll*(nz+2*hs)*(nx+2*hs) + (k+s)*(nx+2*hs) + i+hs;
        stencil[s] = state[inds];
      }
      //Fourth-order-accurate interpolation of the state
      vals[ll] = -stencil[0]/12 + 7*stencil[1]/12 + 7*stencil[2]/12 - stencil[3]/12;
      //First-order-accurate interpolation of the third spatial derivative of the state
      d3_vals[ll] = -stencil[0] + 3*stencil[1] - 3*stencil[2] + stencil[3];
    }

    //Compute density, u-wind, w-wind, potential temperature, and pressure (r,u,w,t,p respectively)
    double r = vals[ID_DENS] + hy_dens_int[k];
    double u = vals[ID_UMOM] / r;
    double w = vals[ID_WMOM] / r;
    double t = ( vals[ID_RHOT] + hy_dens_theta_int[k] ) / r;
    double p = C0*pow((r*t),gamm) - hy_pressure_int[k];
    //Enforce vertical boundary condition and exact mass conservation
    if (k == 0 || k == nz) {
      w                = 0;
      d3_vals[ID_DENS] = 0;
    }

    //Compute the flux vector with hyperviscosity
    flux[ID_DENS*(nz+1)*(nx+1) + k*(nx+1) + i] = r*w     - hv_coef*d3_vals[ID_DENS];
    flux[ID_UMOM*(nz+1)*(nx+1) + k*(nx+1) + i] = r*w*u   - hv_coef*d3_vals[ID_UMOM];
    flux[ID_WMOM*(nz+1)*(nx+1) + k*(nx+1) + i] = r*w*w+p - hv_coef*d3_vals[ID_WMOM];
    flux[ID_RHOT*(nz+1)*(nx+1) + k*(nx+1) + i] = r*w*t   - hv_coef*d3_vals[ID_RHOT];
  }
}

void compute_tend_z (const double * state,
                     const double * flux,
                           double * tend,
                     const int nx,
                     const int nz,
                     const int dz )
{
  int ll = _bid_z * BLOCK_DIM_Z + _tid_z;
  int k = _bid_y * BLOCK_DIM_Y + _tid_y;
  int i = _bid_x * BLOCK_DIM_X + _tid_x;
  if (i < nx && k < nz) {
    int indt  = ll* nz   * nx    + k* nx    + i  ;
    int indf1 = ll*(nz+1)*(nx+1) + (k  )*(nx+1) + i;
    int indf2 = ll*(nz+1)*(nx+1) + (k+1)*(nx+1) + i;
    tend[indt] = -( flux[indf2] - flux[indf1] ) / dz;
    if (ll == ID_WMOM) {
      int inds = ID_DENS*(nz+2*hs)*(nx+2*hs) + (k+hs)*(nx+2*hs) + i+hs;
      tend[indt] = tend[indt] - state[inds]*grav;
    }
  }
}

void pack_send_buf (const double * state,
                          double * sendbuf_l,
                          double * sendbuf_r,
                    const int nx,
                    const int nz,
                    const int hs)
{
  int ll = _bid_z * BLOCK_DIM_Z + _tid_z;
  int k = _bid_y * BLOCK_DIM_Y + _tid_y;
  int s = _bid_x * BLOCK_DIM_X + _tid_x;
  if (s < hs && k < nz) {
    sendbuf_l[ll*nz*hs + k*hs + s] = state[ll*(nz+2*hs)*(nx+2*hs) + (k+hs)*(nx+2*hs) + hs+s];
    sendbuf_r[ll*nz*hs + k*hs + s] = state[ll*(nz+2*hs)*(nx+2*hs) + (k+hs)*(nx+2*hs) + nx+s];
  }
}

void unpack_recv_buf (double * state,
                      const double * recvbuf_l,
                      const double * recvbuf_r,
                      const int nx,
                      const int nz,
                      const int hs)
{
  int ll = _bid_z * BLOCK_DIM_Z + _tid_z;
  int k = _bid_y * BLOCK_DIM_Y + _tid_y;
  int s = _bid_x * BLOCK_DIM_X + _tid_x;
  if (s < hs && k < nz) {
    state[ll*(nz+2*hs)*(nx+2*hs) + (k+hs)*(nx+2*hs) + s      ] = recvbuf_l[ll*nz*hs + k*hs + s];
    state[ll*(nz+2*hs)*(nx+2*hs) + (k+hs)*(nx+2*hs) + nx+hs+s] = recvbuf_r[ll*nz*hs + k*hs + s];
  }
}

void update_state_x (double * state,
                     const double * hy_dens_cell,
                     const double * hy_dens_theta_cell,
                     const int nx,
                     const int nz,
                     const int hs,
                     const int k_beg,
                     const double dz)
{
  int k = _bid_y * BLOCK_DIM_Y + _tid_y;
  int i = _bid_x * BLOCK_DIM_X + _tid_x;
  if (i < hs && k < nz) {
    double z = (k_beg + k+0.5)*dz;
    if (fabs(z-3*zlen/4) <= zlen/16) {
      int ind_r = ID_DENS*(nz+2*hs)*(nx+2*hs) + (k+hs)*(nx+2*hs) + i;
      int ind_u = ID_UMOM*(nz+2*hs)*(nx+2*hs) + (k+hs)*(nx+2*hs) + i;
      int ind_t = ID_RHOT*(nz+2*hs)*(nx+2*hs) + (k+hs)*(nx+2*hs) + i;
      state[ind_u] = (state[ind_r]+hy_dens_cell[k+hs]) * 50.;
      state[ind_t] = (state[ind_r]+hy_dens_cell[k+hs]) * 298. - hy_dens_theta_cell[k+hs];
    }
  }
}

void update_state_z (double * state,
                     const int data_spec_int,
                     const int i_beg,
                     const int nx,
                     const int nz,
                     const int hs,
                     const double dx,
                     const double mnt_width)
{
  int ll = _bid_y * BLOCK_DIM_Y + _tid_y;
  int i = _bid_x * BLOCK_DIM_X + _tid_x;
  if (i < nx+2*hs && ll < NUM_VARS) {
    if (ll == ID_WMOM) {
      state[ll*(nz+2*hs)*(nx+2*hs) + (0      )*(nx+2*hs) + i] = 0.;
      state[ll*(nz+2*hs)*(nx+2*hs) + (1      )*(nx+2*hs) + i] = 0.;
      state[ll*(nz+2*hs)*(nx+2*hs) + (nz+hs  )*(nx+2*hs) + i] = 0.;
      state[ll*(nz+2*hs)*(nx+2*hs) + (nz+hs+1)*(nx+2*hs) + i] = 0.;
      //Impose the vertical momentum effects of an artificial cos^2 mountain at the lower boundary
      if (data_spec_int == DATA_SPEC_MOUNTAIN) {
        double x = (i_beg+i-hs+0.5)*dx;
        if ( fabs(x-xlen/4) < mnt_width ) {
          double xloc = (x-(xlen/4)) / mnt_width;
          //Compute the derivative of the fake mountain
          double mnt_deriv = -pi*cos(pi*xloc/2)*sin(pi*xloc/2)*10/dx;
          //w = (dz/dx)*u
          state[ID_WMOM*(nz+2*hs)*(nx+2*hs) + (0)*(nx+2*hs) + i] = mnt_deriv*state[ID_UMOM*(nz+2*hs)*(nx+2*hs) + hs*(nx+2*hs) + i];
          state[ID_WMOM*(nz+2*hs)*(nx+2*hs) + (1)*(nx+2*hs) + i] = mnt_deriv*state[ID_UMOM*(nz+2*hs)*(nx+2*hs) + hs*(nx+2*hs) + i];
        }
      }
    } else {
      state[ll*(nz+2*hs)*(nx+2*hs) + (0      )*(nx+2*hs) + i] = state[ll*(nz+2*hs)*(nx+2*hs) + (hs     )*(nx+2*hs) + i];
      state[ll*(nz+2*hs)*(nx+2*hs) + (1      )*(nx+2*hs) + i] = state[ll*(nz+2*hs)*(nx+2*hs) + (hs     )*(nx+2*hs) + i];
      state[ll*(nz+2*hs)*(nx+2*hs) + (nz+hs  )*(nx+2*hs) + i] = state[ll*(nz+2*hs)*(nx+2*hs) + (nz+hs-1)*(nx+2*hs) + i];
      state[ll*(nz+2*hs)*(nx+2*hs) + (nz+hs+1)*(nx+2*hs) + i] = state[ll*(nz+2*hs)*(nx+2*hs) + (nz+hs-1)*(nx+2*hs) + i];
    }
  }
}

void acc_mass_te (double * mass,
                  double * te,
                  const double * state,
                  const double * hy_dens_cell,
                  const double * hy_dens_theta_cell,
                  const int nx,
                  const int nz,
                  const double dx,
                  const double dz)
{
  int k = _bid_y * BLOCK_DIM_Y + _tid_y;
  int i = _bid_x * BLOCK_DIM_X + _tid_x;
  if (k < nz && i < nx) {
    int ind_r = ID_DENS*(nz+2*hs)*(nx+2*hs) + (k+hs)*(nx+2*hs) + i+hs;
    int ind_u = ID_UMOM*(nz+2*hs)*(nx+2*hs) + (k+hs)*(nx+2*hs) + i+hs;
    int ind_w = ID_WMOM*(nz+2*hs)*(nx+2*hs) + (k+hs)*(nx+2*hs) + i+hs;
    int ind_t = ID_RHOT*(nz+2*hs)*(nx+2*hs) + (k+hs)*(nx+2*hs) + i+hs;
    double r  = state[ind_r] + hy_dens_cell[hs+k];               // Density
    double u  = state[ind_u] / r;                                // U-wind
    double w  = state[ind_w] / r;                                // W-wind
    double th = ( state[ind_t] + hy_dens_theta_cell[hs+k] ) / r; // Potential Temperature (theta)
    double p  = C0*pow(r*th,gamm);                         // Pressure
    double t  = th / pow(p0/p,rd/cp);                      // Temperature
    double ke = r*(u*u+w*w);                                     // Kinetic Energy
    double ie = r*cv*t;                                          // Internal Energy

    // mass += r        *dx*dz; // Accumulate domain mass
    // te   += (ke + ie)*dx*dz; // Accumulate domain total energy
    atomic_add(mass, r*dx*dz);
    atomic_add(te, (ke+ie)*dx*dz);
  }
}

void update_fluid_state (const double * state_init,
                               double * state_out,
                         const double * tend,
                         const int nx,
                         const int nz,
                         const int hs,
                         const double dt)
{
  int ll = _bid_z * BLOCK_DIM_Z + _tid_z;
  int k = _bid_y * BLOCK_DIM_Y + _tid_y;
  int i = _bid_x * BLOCK_DIM_X + _tid_x;
  if (i < nx && k < nz) {
    int inds = ll*(nz+2*hs)*(nx+2*hs) + (k+hs)*(nx+2*hs) + i+hs;
    int indt = ll*nz*nx + k*nx + i;
    state_out[inds] = state_init[inds] + dt * tend[indt];
  }
}
