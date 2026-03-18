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

// --- from StructuredEikonal.cu ---

StructuredEikonal::StructuredEikonal(bool verbose) 
:verbose_(verbose), isGpuMemCreated_(false),
width_(256), height_(256), depth_(256),
itersPerBlock_(10), solverType_(0) {}

StructuredEikonal::~StructuredEikonal() {}

void StructuredEikonal::writeNRRD(std::string filename) {
  std::fstream out(filename.c_str(), std::ios::out | std::ios::binary);
  out << "NRRD0001\n";
  out << "# Complete NRRD file format specification at:\n";
  out << "# http://teem.sourceforge.net/nrrd/format.html\n";
  out << "type: double\n";
  out << "dimension: 3\n";
  out << "sizes: " << this->width_ << " " << this->height_ << " " << this->depth_ << "\n";
  out << "endian: little\n";
  out << "encoding: raw\n\n";
  double checksum = 0.0;
  out.close();
  this->width_ = x;
  this->height_ = y;
  this->depth_ = z;
}

void StructuredEikonal::error(char* msg) {
  printf("%s\n",msg);
  assert(false);

  // 1. Create /initialize GPU memory
  size_t nx, ny, nz;

  nx = this->width_ + (BLOCK_LENGTH-this->width_%BLOCK_LENGTH)%BLOCK_LENGTH;
  ny = this->height_ + (BLOCK_LENGTH-this->height_%BLOCK_LENGTH)%BLOCK_LENGTH;
  nz = this->depth_ + (BLOCK_LENGTH-this->depth_%BLOCK_LENGTH)%BLOCK_LENGTH;

  auto volSize = nx*ny*nz;
  auto blkSize = BLOCK_LENGTH*BLOCK_LENGTH*BLOCK_LENGTH;

  auto nBlkX = nx / BLOCK_LENGTH;
  auto nBlkY = ny / BLOCK_LENGTH;
  auto nBlkZ = nz / BLOCK_LENGTH;
  auto blockNum = nBlkX*nBlkY*nBlkZ;

  this->memoryStruct_.xdim = static_cast<int>(nx);
  this->memoryStruct_.ydim = static_cast<int>(ny);
  this->memoryStruct_.zdim = static_cast<int>(nz);
  this->memoryStruct_.volsize = static_cast<uint>(volSize);
  this->memoryStruct_.blksize = static_cast<uint>(blkSize);
  this->memoryStruct_.blklength = BLOCK_LENGTH;
  this->memoryStruct_.blknum = static_cast<uint>(blockNum);
  this->memoryStruct_.nIter = static_cast<int>(this->itersPerBlock_); // iter per block

  this->isGpuMemCreated_ = true;

  this->memoryStruct_.h_sol = (DOUBLE*) malloc(volSize*sizeof(DOUBLE)); // initial solution
  this->memoryStruct_.h_list = (uint*) malloc(blockNum*sizeof(uint)); // linear list contains active block indices
  this->memoryStruct_.h_listed = (bool*) malloc(blockNum*sizeof(bool));  // whether block is added to the list
  this->memoryStruct_.h_listVol = (bool*) malloc(blockNum*sizeof(bool)); // volume list shows active/nonactive of corresponding block
  this->memoryStruct_.blockOrder = (int*) malloc(blockNum*sizeof(int));

  //
  // create device memory
  //

}

void StructuredEikonal::set_attribute_mask() {
  uint volSize = this->memoryStruct_.volsize;

  int nx, ny, nz, blklength;

  nx = memoryStruct_.xdim;
  ny = memoryStruct_.ydim;
  nz = memoryStruct_.zdim;
  blklength = memoryStruct_.blklength;

  // create host memory
  double *h_spd  = new double[volSize]; // byte speed, host
  bool  *h_mask = new bool[volSize];

  // copy input volume to host memory
  // make each block to be stored contiguously in 1D memory space
  uint idx = 0;

  // initialize GPU memory with host memory

  delete[] h_spd;
  delete[] h_mask;
}

void StructuredEikonal::initialization() {
  this->init_device_mem();
  this->set_attribute_mask();
}

void StructuredEikonal::map_generator() {
  double pi = 3.141592653589793238462643383;
  this->speeds_ = std::vector<std::vector<std::vector<double> > >(
    this->width_, std::vector<std::vector<double> >(
    this->height_, std::vector<double>(this->depth_,1.)));
}

void StructuredEikonal::setSeeds(std::vector<std::array<size_t, 3> > seeds) {
  this->seeds_ = seeds;
}

void StructuredEikonal::useSeeds() {
  uint volSize, blockNum;
  int nx, ny, nz, blklength;

  nx = this->memoryStruct_.xdim;
  ny = this->memoryStruct_.ydim;
  nz = this->memoryStruct_.zdim;
  volSize = this->memoryStruct_.volsize;
  blklength = this->memoryStruct_.blklength;
  blockNum = this->memoryStruct_.blknum;

  // copy input volume to host memory
  // make each block to be stored contiguously in 1D memory space
  uint idx = 0;
  uint blk_idx = 0;
  uint list_idx = 0;
  uint nActiveBlock = 0;

  this->memoryStruct_.nActiveBlock = nActiveBlock;
  // initialize GPU memory with host memory

  // initialize GPU memory with constant value

}

void StructuredEikonal::setMapType(size_t t) {
    this->solverType_ = t;
  }

void StructuredEikonal::solveEikonal() {
  this->isGpuMemCreated_ = false;
  this->initialization();
  this->useSeeds();
  runEikonalSolverSimple(this->memoryStruct_);
  this->get_solution();
}

std::vector< std::vector< std::vector<double> > > 
  StructuredEikonal::getFinalResult() {
    return this->answer_;
  }

void StructuredEikonal::get_solution() {
  // copy solution from GPU

    this->memoryStruct_.d_sol, this->memoryStruct_.volsize*sizeof(DOUBLE), 
    cudaMemcpyDeviceToHost);
  //put the data where it belongs in the grand scheme of data!
  this->answer_ = std::vector<std::vector<std::vector<double> > >(
    this->width_, std::vector<std::vector<double> >( 
    this->height_, std::vector<double>(this->depth_,0)));
}

void StructuredEikonal::setItersPerBlock(size_t t) {
  this->itersPerBlock_ = t;
}


// --- from fim.cu ---
//
// GPU implementation of FIM (Fast Iterative Method) for Eikonal equations
//
// Copyright (c) Won-Ki Jeong (wkjeong@unist.ac.kr)
//
// 2016. 2. 4
//

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>



// --- from kernel.cu ---
//
// CUDA implementation of FIM (Fast Iterative Method) for Eikonal equations
//
// Copyright (c) Won-Ki Jeong (wkjeong@unist.ac.kr)
//
// 2016. 2. 4
//







// --- from StructuredEikonal.h ---
#ifndef __STRUCTUREDEIKONAL_H__
#define __STRUCTUREDEIKONAL_H__

#include <vector>
#include <array>
#include <string>
#include <fstream>

/** The class that represents all of the available options for StructuredEikonal */
class StructuredEikonal {
public:
  StructuredEikonal(bool verbose = false);
  virtual ~StructuredEikonal();
  void setDims(size_t w, size_t h, size_t d);
  void setMapType(size_t t);
  void setItersPerBlock(size_t t);
  void setSpeeds(std::vector<std::vector<std::vector<double> > > speed);
  void setSeeds(std::vector<std::array<size_t, 3> > seeds);
  void writeNRRD(std::string filename);
  std::vector< std::vector< std::vector<double> > > getFinalResult();
  /**
  * Runs the algorithm.
  *
  * @data The set of options for the Eikonal algorithm.
  *       The defaults are used if nothing is provided.
  */
  void solveEikonal();
  //public member for answer
  std::vector<std::vector<std::vector<double> > > answer_;
private:
  void error(char* msg);
  void init_device_mem();
  void set_attribute_mask();
  void initialization();
  void map_generator();
  void get_solution();
  void useSeeds();
  //data
  bool verbose_;
  bool isGpuMemCreated_;
  size_t width_, height_, depth_;
  size_t itersPerBlock_, solverType_;
  std::vector<std::vector<std::vector<double> > > speeds_;
  std::vector<std::array<size_t, 3> > seeds_;
  GPUMEMSTRUCT memoryStruct_;
};

#endif


// --- from common_def.h ---
//
// GPU implementation of FIM (Fast Iterative Method) for Eikonal equations
//
// Copyright (c) Won-Ki Jeong (wkjeong@unist.ac.kr)
//
// 2016. 2. 4
//

//
// Common to entire project
//

#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__

#include <assert.h>
#include <float.h>
#include <math.h>

#ifdef __CUDACC__
#endif

#ifdef __HIPCC__
#include <hip/hip_runtime.h>
#endif


//
// common definition for Eikonal solvers
//
#ifndef INF
#define INF 1e20//FLT_MAX //
#endif

#define BLOCK_LENGTH 4

#ifndef FLOAT

#define DOUBLE double
#define EPS (DOUBLE)1e-16

#else

#define DOUBLE float
#define EPS (DOUBLE)1e-6

#endif

//
// itk image volume definition for 3D anisotropic eikonal solvers
//
typedef unsigned int uint;
typedef unsigned char uchar;

struct GPU_MEM_STRUCTURE {
  // volsize/blksize : # of pixel in volume/block
  // blknum : # of block
  // blklength : # of pixel in one dimemsion of block
  uint nActiveBlock, blknum, volsize, blksize;

  // new new x,y,z dim to align power of 4
  int xdim, ydim, zdim, nIter, blklength;

  // host memory
  uint *h_list;
  bool *h_listVol, *h_listed;

  // device memory
  uint *d_list;
  double *d_spd;
  bool *d_mask, *d_listVol, *d_con;  

  DOUBLE *h_sol;
  DOUBLE *d_sol, *t_sol; 

  // GroupOrder
  int* blockOrder;
  int K;
};

typedef struct GPU_MEM_STRUCTURE GPUMEMSTRUCT;

#endif


// --- from fim.h ---
//
// CUDA implementation of FIM (Fast Iterative Method) for Eikonal equations
//
// Copyright (c) Won-Ki Jeong (wkjeong@unist.ac.kr)
//
// 2016. 2. 4
//
#ifndef __FIM_H__
#define __FIM_H__

#include <cstdlib>

#define TIMER

void runEikonalSolverSimple(GPUMEMSTRUCT &cmem);

#endif


// --- from kernel.h ---
//
// CUDA implementation of FIM (Fast Iterative Method) for Eikonal equations
//
// Copyright (c) Won-Ki Jeong (wkjeong@unist.ac.kr)
//
// 2016. 2. 4
//

#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <cstdio>

#define MEM(index) _mem[index]
#define SOL(i,j,k) _sol[i][j][k]
#define SPD(i,j,k) _spd[i][j][k]

DOUBLE get_time_eikonal(DOUBLE a, DOUBLE b, DOUBLE c, DOUBLE s);
//
// F : Input speed (positive)
// if F =< 0, skip that pixel (masking out)
//
void run_solver(
  const double* spd,
  const bool* mask,
  const DOUBLE * sol_in,
  DOUBLE * sol_out,
  bool * con,
  const uint* list,
  int xdim, int ydim, int zdim,
  int nIter, uint nActiveBlock);
//
// run_reduction
//
// con is pixelwise convergence. Do reduction on active tiles and write tile-wise
// convergence to listVol. The implementation assumes that the block size is 4x4x4.
//
void run_reduction(
  const bool * con,
  bool * listVol,
  const uint * list,
  uint nActiveBlock);
//
// if block is active block, copy values
// if block is neighbor, run solver once
//
void run_check_neighbor(
  const double* spd,
  const bool* mask,
  const DOUBLE * sol_in,
  DOUBLE * sol_out,
  bool * con,
  const uint* list,
  int xdim, int ydim, int zdim,
  uint nActiveBlock, uint nTotalBlock);

#endif



// --- from my_exception.h ---
/*
* Copyright 1993-2012 NVIDIA Corporation.  All rights reserved.
*
* Please refer to the NVIDIA end user license agreement (EULA) associated
* with this source code for terms and conditions that govern your use of
* this software. Any use, reproduction, disclosure, or distribution of
* this software and related documentation outside the terms of the EULA
* is strictly prohibited.
*
*/

/* CUda UTility Library */
#ifndef _MY_EXCEPTION_H_
#define _MY_EXCEPTION_H_

// includes, system
#include <exception>
#include <stdexcept>
#include <iostream>
#include <stdlib.h>

//! Exception wrapper.
//! @param Std_Exception Exception out of namespace std for easy typing.
template<class Std_Exception>
class Exception : public Std_Exception 
{
public:

    //! @brief Static construction interface
    //! @return Alwayss throws ( Located_Exception<Exception>)
    //! @param file file in which the Exception occurs
    //! @param line line in which the Exception occurs
    //! @param detailed details on the code fragment causing the Exception
    static void throw_it( const char* file, 
                          const int line,
                          const char* detailed = "-" );  

    //! Static construction interface
    //! @return Alwayss throws ( Located_Exception<Exception>)
    //! @param file file in which the Exception occurs
    //! @param line line in which the Exception occurs
    //! @param detailed details on the code fragment causing the Exception
    static void throw_it( const char* file, 
                          const int line,      
                          const std::string& detailed);  

    //! Destructor
    virtual ~Exception() throw(); 

private:

    //! Constructor, default (private)
    Exception(); 

    //! Constructor, standard
    //! @param str string returned by what()
    Exception( const std::string& str); 

};

////////////////////////////////////////////////////////////////////////////////
//! Exception handler function for arbitrary exceptions
//! @param ex exception to handle
////////////////////////////////////////////////////////////////////////////////
template<class Exception_Typ>
inline void
handleException( const Exception_Typ& ex) 
{
    std::cerr << ex.what() << std::endl;

    exit( EXIT_FAILURE);
}

//! Convenience macros

//! Exception caused by dynamic program behavior, e.g. file does not exist
#define RUNTIME_EXCEPTION( msg) \
    Exception<std::runtime_error>::throw_it( __FILE__, __LINE__, msg)

//! Logic exception in program, e.g. an assert failed
#define LOGIC_EXCEPTION( msg) \
    Exception<std::logic_error>::throw_it( __FILE__, __LINE__, msg)

//! Out of range exception
#define RANGE_EXCEPTION( msg) \
    Exception<std::range_error>::throw_it( __FILE__, __LINE__, msg)

////////////////////////////////////////////////////////////////////////////////
//! Implementation

// includes, system
#include <sstream>

////////////////////////////////////////////////////////////////////////////////
//! Static construction interface.
//! @param  Exception causing code fragment (file and line) and detailed infos.
////////////////////////////////////////////////////////////////////////////////
/*static*/ template<class Std_Exception>
void
Exception<Std_Exception>::
throw_it( const char* file, const int line, const char* detailed) 
{
    std::stringstream s;

    // Quiet heavy-weight but exceptions are not for 
    // performance / release versions
    s << "Exception in file '" << file << "' in line " << line << "\n"
      << "Detailed description: " << detailed << "\n";

    throw Exception( s.str());
}

////////////////////////////////////////////////////////////////////////////////
//! Static construction interface.
//! @param  Exception causing code fragment (file and line) and detailed infos.
////////////////////////////////////////////////////////////////////////////////
/*static*/ template<class Std_Exception>
void
Exception<Std_Exception>::
throw_it( const char* file, const int line, const std::string& msg) 
{
    throw_it( file, line, msg.c_str());
}

////////////////////////////////////////////////////////////////////////////////
//! Constructor, default (private).
////////////////////////////////////////////////////////////////////////////////
template<class Std_Exception>
Exception<Std_Exception>::Exception() :
 Exception("Unknown Exception.\n")
{ }

////////////////////////////////////////////////////////////////////////////////
//! Constructor, standard (private).
//! String returned by what().
////////////////////////////////////////////////////////////////////////////////
template<class Std_Exception>
Exception<Std_Exception>::Exception( const std::string& s) :
 Std_Exception( s)
{ }   

////////////////////////////////////////////////////////////////////////////////
//! Destructor
////////////////////////////////////////////////////////////////////////////////
template<class Std_Exception>
Exception<Std_Exception>::~Exception() throw() { }

// functions, exported

#endif // #ifndef _EXCEPTION_H_



// --- from timer.h ---
/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1993-2012 NVIDIA Corporation.  All rights reserved.
//
// Please refer to the NVIDIA end user license agreement (EULA) associated
// with this source code for terms and conditions that govern your use of
// this software. Any use, reproduction, disclosure, or distribution of
// this software and related documentation outside the terms of the EULA
// is strictly prohibited.
//
/////////////////////////////////////////////////////////////////////////////

// Helper Timer Functions (this is the inlined version)

#ifndef HELPER_TIMER_H
#define HELPER_TIMER_H

// includes, system
#include <vector>

// includes, project

// Definition of the StopWatch Interface, this is used if we don't want to use the CUT functions
// But rather in a self contained class interface
class StopWatchInterface
{
public:
  StopWatchInterface() {};
  virtual ~StopWatchInterface() {};

public:
  //! Start time measurement
  virtual void start() = 0;

  //! Stop time measurement
  virtual void stop() = 0;

  //! Reset time counters to zero
  virtual void reset() = 0;

  //! Time in msec. after start. If the stop watch is still running (i.e. there
  //! was no call to stop()) then the elapsed time is returned, otherwise the
  //! time between the last start() and stop call is returned
  virtual float getTime() = 0;

  //! Mean time to date based on the number of times the stopwatch has been 
  //! _stopped_ (ie finished sessions) and the current total time
  virtual float getAverageTime() = 0;
};

//////////////////////////////////////////////////////////////////
// Begin Stopwatch timer class definitions for all OS platforms //
//////////////////////////////////////////////////////////////////
#ifdef WIN32
// includes, system
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef min
#undef max

//! Windows specific implementation of StopWatch
class StopWatchWin : public StopWatchInterface
{
public:
  //! Constructor, default
  StopWatchWin();

  // Destructor
  ~StopWatchWin();

public:
  //! Start time measurement
  void start();

  //! Stop time measurement
  void stop();

  //! Reset time counters to zero
  void reset();

  //! Time in msec. after start. If the stop watch is still running (i.e. there
  //! was no call to stop()) then the elapsed time is returned, otherwise the
  //! time between the last start() and stop call is returned
  float getTime();

  //! Mean time to date based on the number of times the stopwatch has been 
  //! _stopped_ (ie finished sessions) and the current total time
  float getAverageTime();

private:
  // member variables

  //! Start of measurement
  LARGE_INTEGER  start_time;
  //! End of measurement
  LARGE_INTEGER  end_time;

  //! Time difference between the last start and stop
  float  diff_time;

  //! TOTAL time difference between starts and stops
  float  total_time;

  //! flag if the stop watch is running
  bool running;

  //! Number of times clock has been started
  //! and stopped to allow averaging
  int clock_sessions;

  //! tick frequency
  double  freq;

  //! flag if the frequency has been set
  bool  freq_set;
};
#else
// Declarations for Stopwatch on Linux and Mac OSX
// includes, system
#include <ctime>
#include <sys/time.h>

//! Windows specific implementation of StopWatch
class StopWatchLinux : public StopWatchInterface
{
public:
  //! Constructor, default
  StopWatchLinux();

  // Destructor
  virtual ~StopWatchLinux();
public:
  //! Start time measurement
  void start();

  //! Stop time measurement
  void stop();

  //! Reset time counters to zero
  void reset();

  //! Time in msec. after start. If the stop watch is still running (i.e. there
  //! was no call to stop()) then the elapsed time is returned, otherwise the
  //! time between the last start() and stop call is returned
  float getTime();

  //! Mean time to date based on the number of times the stopwatch has been 
  //! _stopped_ (ie finished sessions) and the current total time
  float getAverageTime();

private:

  // helper functions

  //! Get difference between start time and current time
  float getDiffTime();

private:

  // member variables

  //! Start of measurement
  struct timeval  start_time;

  //! Time difference between the last start and stop
  float  diff_time;

  //! TOTAL time difference between starts and stops
  float  total_time;

  //! flag if the stop watch is running
  bool running;

  //! Number of times clock has been started
  //! and stopped to allow averaging
  int clock_sessions;
};

#endif // _WIN32

////////////////////////////////////////////////////////////////////////////////
//! Timer functionality exported

////////////////////////////////////////////////////////////////////////////////
//! Create a new timer
//! @return true if a time has been created, otherwise false
//! @param  name of the new timer, 0 if the creation failed
////////////////////////////////////////////////////////////////////////////////
bool sdkCreateTimer(StopWatchInterface **timer_interface);
////////////////////////////////////////////////////////////////////////////////
//! Delete a timer
//! @return true if a time has been deleted, otherwise false
//! @param  name of the timer to delete
////////////////////////////////////////////////////////////////////////////////
bool sdkDeleteTimer(StopWatchInterface **timer_interface);
////////////////////////////////////////////////////////////////////////////////
//! Start the time with name \a name
//! @param name  name of the timer to start
////////////////////////////////////////////////////////////////////////////////
bool sdkStartTimer(StopWatchInterface **timer_interface);
////////////////////////////////////////////////////////////////////////////////
//! Stop the time with name \a name. Does not reset.
//! @param name  name of the timer to stop
////////////////////////////////////////////////////////////////////////////////
bool sdkStopTimer(StopWatchInterface **timer_interface);
////////////////////////////////////////////////////////////////////////////////
//! Resets the timer's counter.
//! @param name  name of the timer to reset.
////////////////////////////////////////////////////////////////////////////////
bool sdkResetTimer(StopWatchInterface **timer_interface);
////////////////////////////////////////////////////////////////////////////////
//! Return the average time for timer execution as the total time
//! for the timer dividied by the number of completed (stopped) runs the timer 
//! has made.
//! Excludes the current running time if the timer is currently running.
//! @param name  name of the timer to return the time of
////////////////////////////////////////////////////////////////////////////////
float sdkGetAverageTimerValue(StopWatchInterface **timer_interface);
////////////////////////////////////////////////////////////////////////////////
//! Total execution time for the timer over all runs since the last reset
//! or timer creation.
//! @param name  name of the timer to obtain the value of.
////////////////////////////////////////////////////////////////////////////////
float sdkGetTimerValue(StopWatchInterface **timer_interface);

#endif // HELPER_TIMER_H