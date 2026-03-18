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

// --- from streamcluster_cuda.cu ---
/***********************************************
	streamcluster_cuda.cu
	: parallelized code of streamcluster
	
	- original code from PARSEC Benchmark Suite
	- parallelization with CUDA API has been applied by
	
	Shawn Sang-Ha Lee - sl4ge@virginia.edu
	University of Virginia
	Department of Electrical and Computer Engineering
	Department of Computer Science
	
***********************************************/

using namespace std;

// AUTO-ERROR CHECK FOR ALL CUDA FUNCTIONS
#define CUDA_SAFE_CALL( call) do {										\
   cudaError err = call;												\

#define THREADS_PER_BLOCK 512
#define MAXBLOCKS 65536
#define CUDATIME

// host memory
float *work_mem_h;
float *coord_h;

// device memory
float *work_mem_d;
float *coord_d;
int   *center_table_d;
bool  *switch_membership_d;
Point *p;

static int iter = 0;		// counter for total# of iteration

//=======================================
// Euclidean Distance
//=======================================

//=======================================
// Kernel - Compute Cost
//=======================================

//=======================================
// Allocate Device Memory
//=======================================

//=======================================
// Allocate Host Memory
//=======================================

//=======================================
// Free Device Memory
//=======================================

//=======================================
// Free Host Memory
//=======================================

//=======================================
// pgain Entry - CUDA SETUP + CUDA CALL
//=======================================


// --- from streamcluster_header.cu ---
/************************************************
	streamcluster_cuda_header.cu
	: header file to streamcluster
	
	- original code from PARSEC Benchmark Suite
	- parallelization with CUDA API has been applied by
	
	Sang-Ha (a.k.a Shawn) Lee - sl4ge@virginia.edu
	University of Virginia
	Department of Electrical and Computer Engineering
	Department of Computer Science
	
***********************************************/

#ifndef STREAMCLUSTER_CUDA_HEADER_CU
#define STREAMCLUSTER_CUDA_HEADER_CU

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <sys/resource.h>
#include <limits.h>

#ifdef ENABLE_PARSEC_HOOKS
#include <hooks.h>
#endif

using namespace std;

/* this structure represents a point */
/* these will be passed around to avoid copying coordinates */
typedef struct {
  float weight;
  float *coord;
  long assign;  /* number of point where this one is assigned */
  float cost;  /* cost of that assignment, weight*distance */
} Point;

/* this is the array of points */
typedef struct {
  long num; /* number of points; may not be N if this is a sample */
  int dim;  /* dimensionality */
  Point *p; /* the array itself */
} Points;

struct pkmedian_arg_t
{
  Points* points;
  long kmin;
  long kmax;
  long* kfinal;
  int pid;
  pthread_barrier_t* barrier;
};

class PStream {
public:
  virtual size_t read( float* dest, int dim, int num ) = 0;
  virtual int ferror() = 0;
};

//synthetic stream
class SimStream : public PStream {
public:
  ~SimStream() { 
  }
private:
  long n;
};

class FileStream : public PStream {
public:
  ~FileStream() {
    printf("closing file stream\n");
    fclose(fp);
  }
private:
  FILE* fp;
};

/* function prototypes */
double gettime();
int isIdentical(float*, float*, int);
//static int floatcomp(const void*, const void*);
void shuffle(Points*);
void intshuffle(int*, int);
float waste(float);
float dist(Point, Point, int);
float pspeedy(Points*, float, long, int, pthread_barrier_t*);
float pgain_old(long, Points*, float, long int*, int, pthread_barrier_t*);
float pFL(Points*, int*, int, float, long*, float, long, float, int, pthread_barrier_t*);
int selectfeasible_fast(Points*, int**, int, int, pthread_barrier_t*);
float pkmedian(Points*, long, long, long*, int, pthread_barrier_t*);
int contcenters(Points*);
void copycenters(Points*, Points*, long*, long);
void* localSearchSub(void*);
void localSearch(Points*, long, long, long*);
void outcenterIDs(Points*, long*, char*);
void streamCluster(PStream*, long, long, int, long, long, char*);
float pgain(long, Points*, float, long int*, int, bool*, int*, bool*, bool, double*, double*, double*, double*, double*, double*);
void allocDevMem(int, int, int);
void allocHostMem(int, int, int);
void freeDevMem();
void freeHostMem();

#endif
