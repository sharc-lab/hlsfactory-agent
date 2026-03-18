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
  // create host/device memory
  //
  hipMalloc((void**)&(this->memoryStruct_.d_spd), volSize*sizeof(double));

  hipMalloc((void**)&(this->memoryStruct_.d_sol), volSize*sizeof(DOUBLE));

  hipMalloc((void**)&(this->memoryStruct_.t_sol), volSize*sizeof(DOUBLE));  // temp solution for ping-pong

  hipMalloc((void**)&(this->memoryStruct_.d_con), volSize*sizeof(bool));  // convergence volume

  hipMalloc((void**)&(this->memoryStruct_.d_list), blockNum*sizeof(uint));

  hipMalloc((void**)&(this->memoryStruct_.d_listVol), blockNum*sizeof(bool));

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
  hipMemcpy(memoryStruct_.d_spd, h_spd, volSize*sizeof(double), hipMemcpyHostToDevice);
  hipMemcpy(memoryStruct_.d_mask, h_mask, volSize*sizeof(bool), hipMemcpyHostToDevice);

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
  hipMemcpy(this->memoryStruct_.d_sol, this->memoryStruct_.h_sol, volSize*sizeof(DOUBLE), hipMemcpyHostToDevice);
  hipMemcpy(this->memoryStruct_.t_sol, this->memoryStruct_.h_sol, volSize*sizeof(DOUBLE), hipMemcpyHostToDevice);
  hipMemcpy(this->memoryStruct_.d_list, this->memoryStruct_.h_list, nActiveBlock*sizeof(uint), hipMemcpyHostToDevice);
  hipMemcpy(this->memoryStruct_.d_listVol, this->memoryStruct_.h_listVol, blockNum*sizeof(bool), hipMemcpyHostToDevice);
  // initialize GPU memory with constant value
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
  hipMemcpy(this->memoryStruct_.h_sol,
    this->memoryStruct_.d_sol, this->memoryStruct_.volsize*sizeof(DOUBLE), 
    hipMemcpyDeviceToHost);
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
// HIP implementation of FIM (Fast Iterative Method) for Eikonal equations
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
#include "common_def.h"

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


// --- from fim.h ---
//
// HIP implementation of FIM (Fast Iterative Method) for Eikonal equations
//
// Copyright (c) Won-Ki Jeong (wkjeong@unist.ac.kr)
//
// 2016. 2. 4
//
#ifndef __FIM_H__
#define __FIM_H__

#include <cstdlib>
#include "common_def.h"

#define TIMER

void runEikonalSolverSimple(GPUMEMSTRUCT &cmem);

#endif


// --- from kernel.h ---
//
// HIP implementation of FIM (Fast Iterative Method) for Eikonal equations
//
// Copyright (c) Won-Ki Jeong (wkjeong@unist.ac.kr)
//
// 2016. 2. 4
//

#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <cstdio>
#include "common_def.h"

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

