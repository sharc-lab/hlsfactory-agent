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

// --- from embedded_fehlberg_7_8.cu ---
////////////////////////////////////////////////////////////////////////////////
// File: embedded_fehlberg_7_8.c                                              //
// Routines:                                                                  //
//    Embedded_Fehlberg_7_8                                                   //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Description:                                                              //
//     The Runge-Kutta-Fehlberg method is an adaptive procedure for approxi-  //
//     mating the solution of the differential equation y'(x) = f(x,y) with   //
//     initial condition y(x0) = c.  This implementation evaluates f(x,y)     //
//     thirteen times per step using embedded seventh order and eight order   //
//     Runge-Kutta estimates to estimate the not only the solution but also   //
//     the error.                                                             //
//     The next step size is then calculated using the preassigned tolerance  //
//     and error estimate.                                                    //
//     For step i+1,                                                          //
//        y[i+1] = y[i] +  h * (41/840 * k1 + 34/105 * finavalu_temp[5] + 9/35 * finavalu_temp[6]         //
//                        + 9/35 * finavalu_temp[7] + 9/280 * finavalu_temp[8] + 9/280 finavalu_temp[9] + 41/840 finavalu_temp[10] ) //
//     where                                                                  //
//     k1 = f( x[i],y[i] ),                                                   //
//     finavalu_temp[1] = f( x[i]+2h/27, y[i] + 2h*k1/27),                                  //
//     finavalu_temp[2] = f( x[i]+h/9, y[i]+h/36*( k1 + 3 finavalu_temp[1]) ),                            //
//     finavalu_temp[3] = f( x[i]+h/6, y[i]+h/24*( k1 + 3 finavalu_temp[2]) ),                            //
//     finavalu_temp[4] = f( x[i]+5h/12, y[i]+h/48*(20 k1 - 75 finavalu_temp[2] + 75 finavalu_temp[3])),                //
//     finavalu_temp[5] = f( x[i]+h/2, y[i]+h/20*( k1 + 5 finavalu_temp[3] + 4 finavalu_temp[4] ) ),                    //
//     finavalu_temp[6] = f( x[i]+5h/6, y[i]+h/108*( -25 k1 + 125 finavalu_temp[3] - 260 finavalu_temp[4] + 250 finavalu_temp[5] ) ), //
//     finavalu_temp[7] = f( x[i]+h/6, y[i]+h*( 31/300 k1 + 61/225 finavalu_temp[4] - 2/9 finavalu_temp[5]              //
//                                                            + 13/900 finavalu_temp[6]) )  //
//     finavalu_temp[8] = f( x[i]+2h/3, y[i]+h*( 2 k1 - 53/6 finavalu_temp[3] + 704/45 finavalu_temp[4] - 107/9 finavalu_temp[5]      //
//                                                      + 67/90 finavalu_temp[6] + 3 finavalu_temp[7]) ), //
//     finavalu_temp[9] = f( x[i]+h/3, y[i]+h*( -91/108 k1 + 23/108 finavalu_temp[3] - 976/135 finavalu_temp[4]        //
//                             + 311/54 finavalu_temp[5] - 19/60 finavalu_temp[6] + 17/6 finavalu_temp[7] - 1/12 finavalu_temp[8]) ), //
//     finavalu_temp[10] = f( x[i]+h, y[i]+h*( 2383/4100 k1 - 341/164 finavalu_temp[3] + 4496/1025 finavalu_temp[4]     //
//          - 301/82 finavalu_temp[5] + 2133/4100 finavalu_temp[6] + 45/82 finavalu_temp[7] + 45/164 finavalu_temp[8] + 18/41 finavalu_temp[9]) )  //
//     finavalu_temp[11] = f( x[i], y[i]+h*( 3/205 k1 - 6/41 finavalu_temp[5] - 3/205 finavalu_temp[6] - 3/41 finavalu_temp[7]        //
//                                                   + 3/41 finavalu_temp[8] + 6/41 finavalu_temp[9]) )  //
//     finavalu_temp[12] = f( x[i]+h, y[i]+h*( -1777/4100 k1 - 341/164 finavalu_temp[3] + 4496/1025 finavalu_temp[4]    //
//                      - 289/82 finavalu_temp[5] + 2193/4100 finavalu_temp[6] + 51/82 finavalu_temp[7] + 33/164 finavalu_temp[8] +   //
//                                                        12/41 finavalu_temp[9] + finavalu_temp[11]) )  //
//     x[i+1] = x[i] + h.                                                     //
//                                                                            //
//     The error is estimated to be                                           //
//        err = -41/840 * h * ( k1 + finavalu_temp[10] - finavalu_temp[11] - finavalu_temp[12])                         //
//     The step size h is then scaled by the scale factor                     //
//         scale = 0.8 * | epsilon * y[i] / [err * (xmax - x[0])] | ^ 1/7     //
//     The scale factor is further constrained 0.125 < scale < 4.0.           //
//     The new step size is h := scale * h.                                   //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//  static fp Runge_Kutta(fp (*f)(fp,fp), fp *y,          //
//                                                       fp x0, fp h) //
//                                                                            //
//  Description:                                                              //
//     This routine uses Fehlberg's embedded 7th and 8th order methods to     //
//     approximate the solution of the differential equation y'=f(x,y) with   //
//     the initial condition y = y[0] at x = x0.  The value at x + h is       //
//     returned in y[1].  The function returns err / h ( the absolute error   //
//     per step size ).                                                       //
//                                                                            //
//  Arguments:                                                                //
//     fp *f  Pointer to the function which returns the slope at (x,y) of //
//                integral curve of the differential equation y' = f(x,y)     //
//                which passes through the point (x0,y[0]).                   //
//     fp y[] On input y[0] is the initial value of y at x, on output     //
//                y[1] is the solution at x + h.                              //
//     fp x   Initial value of x.                                         //
//     fp h   Step size                                                   //
//                                                                            //
//  Return Values:                                                            //
//     This routine returns the err / h.  The solution of y(x) at x + h is    //
//     returned in y[1].                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



// --- from kernel_cam.cu ---
//=====================================================================
//  MAIN FUNCTION
//=====================================================================


// --- from kernel_ecc.cu ---
//=====================================================================
//  MAIN FUNCTION                            
//=====================================================================


// --- from kernel_fin.cu ---
//=====================================================================
//  MAIN FUNCTION
//=====================================================================


// --- from main.cu ---
// Lukasz G. Szafaryn 24 JAN 09

// Myocyte application models cardiac myocyte (heart muscle cell) and simulates its behavior according to the work by Saucerman and Bers [8]. The model integrates 
// cardiac myocyte electrical activity with the calcineurin pathway, which is a key aspect of the development of heart failure. The model spans large number of temporal 
// scales to reflect how changes in heart rate as observed during exercise or stress contribute to calcineurin pathway activation, which ultimately leads to the expression 
// of numerous genes that remodel the heart�s structure. It can be used to identify potential therapeutic targets that may be useful for the treatment of heart failure. 
// Biochemical reactions, ion transport and electrical activity in the cell are modeled with 91 ordinary differential equations (ODEs) that are determined by more than 200 
// experimentally validated parameters. The model is simulated by solving this group of ODEs for a specified time interval. The process of ODE solving is based on the 
// causal relationship between values of ODEs at different time steps, thus it is mostly sequential. At every dynamically determined time step, the solver evaluates the 
// model consisting of a set of 91 ODEs and 480 supporting equations to determine behavior of the system at that particular time instance. If evaluation results are not 
// within the expected tolerance at a given time step (usually as a result of incorrect determination of the time step), another calculation attempt is made at a modified 
// (usually reduced) time step. Since the ODEs are stiff (exhibit fast rate of change within short time intervals), they need to be simulated at small time scales with an 
// adaptive step size solver. 

//  1) The original version of the current solver code was obtained from: Mathematics Source Library (http://mymathlib.webtrellis.net/index.html). The solver has been 
//      somewhat modified to tailor it to our needs. However, it can be reverted back to original form or modified to suit other simulations.
// 2) This solver and particular solving algorithm used with it (embedded_fehlberg_7_8) were adapted to work with a set of equations, not just one like in original version.
//  3) In order for solver to provide deterministic number of steps (needed for particular amount of memore previousely allocated for results), every next step is 
//      incremented by 1 time unit (h_init).
//  4) Function assumes that simulation starts at some point of time (whatever time the initial values are provided for) and runs for the number of miliseconds (xmax) 
//      specified by the uses as a parameter on command line.
// 5) The appropriate amount of memory is previousely allocated for that range (y).
//  6) This setup in 3) - 5) allows solver to adjust the step ony from current time instance to current time instance + 0.9. The next time instance is current time instance + 1;
//  7) The original solver cannot handle cases when equations return NAN and INF values due to discontinuities and /0. That is why equations provided by user need to 
//      make sure that no NAN and INF are returned.
// 8) Application reads initial data and parameters from text files: y.txt and params.txt respectively that need to be located in the same folder as source files. 
//     For simplicity and testing purposes only, when multiple number of simulation instances is specified, application still reads initial data from the same input files. That 
//     can be modified in this source code.

//    IMPLEMENTATION-SPECIFIC DESCRIPTION

// The original single-threaded code was written in MATLAB and used MATLAB ode45 ODE solver. In the process of accelerating this code, we arrived with the 
// intermediate versions that used single-threaded Sundials CVODE solver which evaluated model parallelized with CUDA at each time step. In order to convert entire 
// solver to CUDA code (to remove some of the operational overheads such as kernel launches and data transfer in CUDA) we used a simpler solver, from Mathematics 
// Source Library, and tailored it to our needs. The parallelism in the cardiac myocyte model is on a very fine-grained level, close to that of ILP, therefore it is very hard 
// to exploit as DLP or TLB in CUDA code. We were able to divide the model into 4 individual groups that run in parallel. However, even that is not enough work to 
// compensate for some of the CUDA thread launch and data transfer overheads which resulted in performance worse than that of single-threaded C code. Speedup in 
// this code could be achieved only if a customizable accelerator such as FPGA was used for evaluation of the model itself. We also approached the application from 
// another angle and allowed it to run several concurrent simulations, thus turning it into an embarrassingly parallel problem. This version of the code is also useful for 
// scientists who want to run the same simulation with different sets of input parameters. Speedup achieved with CUDA code is variable on the other hand. It depends on 
// the number of concurrent simulations and it saturates around 300 simulations with the speedup of about 10x.

// Speedup numbers reported in the description of this application were obtained on the machine with: Intel Quad Core CPU, 4GB of RAM, Nvidia GTX280 GPU.  

// 1) When running with parallelization inside each simulation instance (value of 3rd command line parameter equal to 0), performance is bad because:
// a) underutilization of GPU (only 1 out of 32 threads in each block)
// b) significant CPU-GPU memory copy overhead
// c) kernel launch overhead (kernel needs to be finished every time model is evaluated as it is the only way to synchronize threads in different blocks)
// 2) When running with parallelization across simulation instances, code gets continues speedup with the increasing number of simulation insances which saturates
//     around 240 instances on GTX280 (roughly corresponding to the number of multiprocessorsXprocessors in GTX280), with the speedup of around 10x compared
//     to serial C version of code. Limited performance is explained mainly by:
// a) significant CPU-GPU memory copy overhead
// b) increasingly uncoalesced memory accesses with the increasing number of workloads
// c) lack of cache compared to CPU, or no use of shared memory to compensate
// d) frequency of GPU shader being lower than that of CPU core
// 3) GPU version has an issue with memory allocation that has not been resolved yet. For certain simulation ranges memory allocation fails, or pointers incorrectly overlap 
//     causeing value trashing.

// The following are the command parameters to the application:
// 1) Simulation time interval which is the number of miliseconds to simulate. Needs to be integer > 0
// 2) Number of instances of simulation to run. Needs to be integer > 0.
// 3) Method of parallelization. Need to be 0 for parallelization inside each simulation instance, or 1 for parallelization across instances.
// Example:
// a.out 100 100 1

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <chrono>
#include "file.c"



// --- from master.cu ---
//=====================================================================
//  MAIN FUNCTION
//=====================================================================



// --- from solver.cu ---
////////////////////////////////////////////////////////////////////////////////
// int solver( fp (*f)(fp, fp), fp y[],        //
//       fp x, fp h, fp xmax, fp *h_next, fp tolerance )  //
//                                                                            //
//  Description:                                                              //
//     This function solves the differential equation y'=f(x,y) with the      //
//     initial condition y(x) = y[0].  The value at xmax is returned in y[1]. //
//     The function returns 0 if successful or -1 if it fails.                //
//                                                                            //
//  Arguments:                                                                //
//     fp *f  Pointer to the function which returns the slope at (x,y) of //
//                integral curve of the differential equation y' = f(x,y)     //
//                which passes through the point (x0,y0) corresponding to the //
//                initial condition y(x0) = y0.                               //
//     fp y[] On input y[0] is the initial value of y at x, on output     //
//                y[1] is the solution at xmax.                               //
//     fp x   The initial value of x.                                     //
//     fp h   Initial step size.                                          //
//     fp xmax The endpoint of x.                                         //
//     fp *h_next   A pointer to the estimated step size for successive   //
//                      calls to solver.                       //
//     fp tolerance The tolerance of y(xmax), i.e. a solution is sought   //
//                so that the relative error < tolerance.                     //
//                                                                            //
//  Return Values:                                                            //
//     0   The solution of y' = f(x,y) from x to xmax is stored y[1] and      //
//         h_next has the value to the next size to try.                      //
//    -1   The solution of y' = f(x,y) from x to xmax failed.                 //
//    -2   Failed because either xmax < x or the step size h <= 0.            //
//    -3   Memory limit allocated for results was reached                                //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//  Summary of changes by Lukasz G. Szafaryn:

//  1) The original code was obtained from: Mathematics Source Library (http://mymathlib.webtrellis.net/index.html)
// 2) This solver and particular solving algorithm used with it (embedded_fehlberg_7_8) were adapted to work with a set of equations, not just one like in original version.

//  3) In order for solver to provide deterministic number of steps (needed for particular amount of memore previousely allocated for results), every next step is incremented by 1 time unit (h_init).
//  4) Function assumes that time interval starts at 0 (xmin) and ends at integer value (xmax) specified by the uses as a parameter on command line.
// 5) The appropriate amount of memory is previousely allocated for that range (y).

//  5) This setup in 3) - 5) allows solver to adjust the step ony from current time instance to current time instance + 0.9. The next time instance is current time instance + 1;

//  6) Solver also takes parameters (params) that it then passes to the equations.

//  7) The original solver cannot handle cases when equations return NAN and INF values due to discontinuities and /0. That is why equations provided by user need to make sure that no NAN and INF are returned.

//  Last update: 15 DEC 09
////////////////////////////////////////////////////////////////////////////////

//======================================================================================================================================================
//======================================================================================================================================================
//    INCLUDE
//======================================================================================================================================================
//======================================================================================================================================================

#include <math.h>

#define max(x,y) ( (x) < (y) ? (y) : (x) )
#define min(x,y) ( (x) < (y) ? (x) : (y) )

#define ATTEMPTS 12
#define MIN_SCALE_FACTOR 0.125
#define MAX_SCALE_FACTOR 4.0



// --- from define.h ---
//	DEFINE / INCLUDE

#define fp float

#define NUMBER_THREADS 32

#define EQUATIONS 91
#define PARAMETERS 18
