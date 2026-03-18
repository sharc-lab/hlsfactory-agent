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

// --- from streamcluster.cu ---
/***********************************************
  streamcluster.cpp
  : original source code of streamcluster with minor
  modification regarding function calls

  - original code from PARSEC Benchmark Suite
  - parallelization with CUDA API has been applied by

  Sang-Ha (a.k.a Shawn) Lee - sl4ge@virginia.edu
  University of Virginia
  Department of Electrical and Computer Engineering
  Department of Computer Science
  :revised by
  Jianbin Fang - j.fang@tudelft.nl
  Delft University of Technology
  Faculty of Electrical Engineering, Mathematics and Computer Science
  Department of Software Technology
  Parallel and Distributed Systems Group
  on 15/03/2011
 ***********************************************/


using namespace std;

#define MAXNAMESIZE 1024   // max filename length
#define SEED 1
/* increase this to reduce probability of random error */
/* increasing it also ups running time of "speedy" part of the code */
/* SP = 1 seems to be fine */
#define SP 1               // number of repetitions of speedy must be >=1

/* higher ITER --> more likely to get correct # of centers */
/* higher ITER also scales the running time almost linearly */
#define ITER 3             // iterate ITER* k log k times; ITER >= 1

//#define PRINTINFO       //comment this out to disable output
//#define PROFILE_TMP           // comment this out to disable instrumentation code
//#define ENABLE_THREADS  // comment this out to disable threads
//#define INSERT_WASTE     //uncomment this to insert waste computation into dist function

#define CACHE_LINE 512     // cache line in byte

/* global */
static char *switch_membership;  //whether to switch membership in pgain
static bool *is_center;            //whether a point is a center
static int  *center_table;          //index table of centers
static int nproc;                 //# of threads

/* timing info */
static double serial;
static double cpu_gpu_memcpy;
static double memcpy_back;
static double gpu_malloc;
static double kernel_time;
static int cnt_speedy;

// instrumentation code
#ifdef PROFILE_TMP
static double gpu_free;
double time_local_search;
double time_speedy;
double time_select_feasible;
double time_gain;
double time_shuffle;
double time_gain_dist;
double time_gain_init;
double time_FL;
#endif 




/* comparator for floating point numbers 

/* shuffle points into random order */

/* shuffle an array of integers */

#ifdef INSERT_WASTE
#endif

/* compute Euclidean distance squared between two points */

/* run speedy on the points, return total cost of solution */

/* facility location on the points using local search */
/* z is the facility cost, returns the total cost and # of centers */
/* assumes we are seeded with a reasonable solution */
/* cost should represent this solution's cost */
/* halt if there is < e improvement after iter calls to gain */
/* feasible is an array of numfeasible points which may be centers */



/* compute approximate kmedian on the points */

/* compute the means for the k clusters */

/* copy centers from points to centers */







// --- from kernel.h ---
void compute_cost(
    const Point_Struct * p_d_acc,       
    const float * coord_d_acc,
          float * work_mem_d_acc,      
    const int * center_table_d_acc,
    char * switch_membership_d_acc,      
    const int num,
    const int dim,
    const long x,
    const int K)
{  
  float coord_s_acc[4096]; 
  /* block ID and global thread ID */
  const int local_id = _tid_x; 
  const int thread_id = BLOCK_DIM_X*_bid_x+local_id;

  if(thread_id<num){
    // coordinate mapping of point[x] to shared mem
    if(local_id == 0)
      for(int i=0; i<dim; i++){ 
        coord_s_acc[i] = coord_d_acc[i*num + x];
      }

    // cost between this point and point[x]: euclidean distance multiplied by weight
    float x_cost = 0.0;
    for(int i=0; i<dim; i++)
      x_cost += (coord_d_acc[(i*num)+thread_id]-coord_s_acc[i]) * (coord_d_acc[(i*num)+thread_id]-coord_s_acc[i]);
    x_cost = x_cost * p_d_acc[thread_id].weight;

    float current_cost = p_d_acc[thread_id].cost;

    int base = thread_id*(K+1);   
    // if computed cost is less then original (it saves), mark it as to reassign    
    if ( x_cost < current_cost ){
      switch_membership_d_acc[thread_id] = '1';
      int addr_1 = base + K;
      work_mem_d_acc[addr_1] = x_cost - current_cost;
    }
    // if computed cost is larger, save the difference
    else {
      int assign = p_d_acc[thread_id].assign;
      int addr_2 = base + center_table_d_acc[assign];
      work_mem_d_acc[addr_2] += current_cost - x_cost;
    }
  }
}


// --- from streamcluster.h ---
/***********************************************
	streamcluster.h
	: header file to streamcluster
	
	- original code from PARSEC Benchmark Suite
	- parallelization with CUDA API has been applied by
	
	Sang-Ha (a.k.a Shawn) Lee - sl4ge@virginia.edu
	University of Virginia
	Department of Electrical and Computer Engineering
	Department of Computer Science
	
	:revised by
	Jianbin Fang - j.fang@tudelft.nl
	Delft University of Technology
	Faculty of Electrical Engineering, Mathematics and Computer Science
	Department of Software Technology
	Parallel and Distributed Systems Group
	on 15/03/2011
***********************************************/

#ifndef STREAMCLUSTER_H
#define STREAMCLUSTER_H

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
  float * coord;
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
  virtual int feof() = 0;
  virtual ~PStream() {
  }
};

//synthetic stream
class SimStream : public PStream {
public:
  SimStream(long n_ ) {
    n = n_;
  }
  size_t read( float* dest, int dim, int num ) {
    size_t count = 0;
    for( int i = 0; i < num && n > 0; i++ ) {
      for( int k = 0; k < dim; k++ ) {
	dest[i*dim + k] = lrand48()/(float)INT_MAX;
      }
      n--;
      count++;
    }
    return count;
  }
  int ferror() {
    return 0;
  }
  int feof() {
    return n <= 0;
  }
  ~SimStream() { 
  }
private:
  long n;
};

class FileStream : public PStream {
public:
  FileStream(char* filename) {
    fp = fopen( filename, "rb");
    if( fp == NULL ) {
      fprintf(stderr,"error opening file %s\n.",filename);
      exit(1);
    }
  }
  size_t read( float* dest, int dim, int num ) {
    return std::fread(dest, sizeof(float)*dim, num, fp); 
  }
  int ferror() {
    return std::ferror(fp);
  }
  int feof() {
    return std::feof(fp);
  }
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
float pgain(long, Points*, float, long int*, int, bool*, int*, bool*, double*, double*, double*, double*, double*);
void allocDevMem(int, int, int);
void freeDevMem();

#endif


// --- from streamcluster_cl.h ---
/***********************************************
  streamcluster_cl.h
  : parallelized code of streamcluster

  - original code from PARSEC Benchmark Suite
  - parallelization with OpenCL API has been applied by
  Jianbin Fang - j.fang@tudelft.nl
  Delft University of Technology
  Faculty of Electrical Engineering, Mathematics and Computer Science
  Department of Software Technology
  Parallel and Distributed Systems Group
  on 15/03/2010
 ***********************************************/

#define THREADS_PER_BLOCK 256
#define MAXBLOCKS 65536

typedef struct {
  float weight;
  long assign;  /* number of point where this one is assigned */
  float cost;  /* cost of that assignment, weight*distance */
} Point_Struct;

// GPU kernel 

/* host memory analogous to device memory. These memories are allocated in the function,
 * but they are freed in the streamcluster.cpp. We cannot free them in the function as
 * the funtion is called repeatedly in streamcluster.cpp. */
float *work_mem_h;
float *coord_h;
float *gl_lower;
Point_Struct *p_h;

// device memory
float *work_mem_d;
float *coord_d;
int   *center_table_d;
char  *switch_membership_d;
Point_Struct *p_d;

static int c;      // counters

float pgain( long x, Points *points, float z, long int *numcenters, 
             int kmax, bool *is_center, int *center_table, char *switch_membership,
             double *serial, double *cpu_gpu_memcpy, double *memcpy_back,
             double *gpu_malloc, double *kernel_time) {

  float gl_cost = 0;
  try{
#ifdef PROFILE_TMP
    double t1 = gettime();
#endif
    int K = *numcenters ;   // number of centers
    int num = points->num;  // number of points
    int dim = points->dim;  // number of dimensions
    kmax++;
    /***** build center index table 1*****/
    int count = 0;
    for( int i=0; i<num; i++){
      if( is_center[i] )
        center_table[i] = count++;
    }

#ifdef PROFILE_TMP
    double t2 = gettime();
    *serial += t2 - t1;
#endif

    /***** initial memory allocation and preparation for transfer : execute once *****/
    if( c == 0 ) {
#ifdef PROFILE_TMP
      double t3 = gettime();
#endif
      coord_h = (float*) malloc( num * dim * sizeof(float)); // coordinates (host)
      gl_lower = (float*) malloc( kmax * sizeof(float) );
      work_mem_h = (float*) malloc (kmax*num*sizeof(float));
      p_h = (Point_Struct*) malloc (num*sizeof(Point_Struct));

      // prepare mapping for point coordinates
      for(int i=0; i<dim; i++){
        for(int j=0; j<num; j++)
          coord_h[ (num*i)+j ] = points->p[j].coord[i];
      }
#ifdef PROFILE_TMP    
      double t4 = gettime();
      *serial += t4 - t3;
#endif

#ifdef PROFILE_TMP
      double t5 = gettime();
      *gpu_malloc += t5 - t4;
#endif

      // copy coordinate to device memory  
      cudaMemcpyAsync(coord_d, coord_h, sizeof(float)*num*dim, cudaMemcpyHostToDevice, 0);

#ifdef PROFILE_TMP

      double t6 = gettime();
      *cpu_gpu_memcpy += t6 - t4;
#endif
    }    // first iteration

#ifdef PROFILE_TMP
    double t100 = gettime();
#endif

    for(int i=0; i<num; i++){
      p_h[i].weight = ((points->p)[i]).weight;
      p_h[i].assign = ((points->p)[i]).assign;
      p_h[i].cost = ((points->p)[i]).cost;  
    }

#ifdef PROFILE_TMP
    double t101 = gettime();
    *serial += t101 - t100;
#endif
#ifdef PROFILE_TMP
    double t7 = gettime();
#endif
    /***** memory transfer from host to device *****/
    cudaMemcpyAsync(center_table_d, center_table, sizeof(int)*num, cudaMemcpyHostToDevice, 0);
    cudaMemcpyAsync(p_d, p_h, sizeof(Point_Struct)*num, cudaMemcpyHostToDevice, 0);

#ifdef PROFILE_TMP

    double t8 = gettime();
    *cpu_gpu_memcpy += t8 - t7;
#endif

    /***** kernel execution *****/
    /* Determine the number of thread blocks in the x- and y-dimension */
    size_t smSize = dim;

#ifdef PROFILE_TMP
    double t9 = gettime();
#endif

    cudaMemsetAsync(switch_membership_d, 0, num * sizeof(char), 0);

    cudaMemsetAsync(work_mem_d, 0, num*(K+1)*sizeof(float), 0);

    int work_group_size = THREADS_PER_BLOCK;
    int work_items = num;
    if(work_items%work_group_size != 0)  //process situations that work_items cannot be divided by work_group_size
      work_items = work_items + (work_group_size-(work_items%work_group_size));

      p_d, coord_d, work_mem_d, center_table_d, switch_membership_d,
      num, dim, x, K);

#ifdef PROFILE_TMP

    double t10 = gettime();
    *kernel_time += t10 - t9;
#endif

    /***** copy back to host for CPU side work *****/

#ifdef PROFILE_TMP
    double t11 = gettime();
    *memcpy_back += t11 - t10;
#endif

    /****** cpu side work *****/
    int numclose = 0;
    gl_cost = z;

    /* compute the number of centers to close if we are to open i */
    for(int i=0; i < num; i++){
      if( is_center[i] ) {
        float low = z;
        //printf("i=%d  ", i);
        for( int j = 0; j < num; j++ )
          low += work_mem_h[ j*(K+1) + center_table[i] ];
        //printf("low=%f\n", low);    
        gl_lower[center_table[i]] = low;

        if ( low > 0 ) {
          numclose++;        
          work_mem_h[i*(K+1)+K] -= low;
        }
      }
      gl_cost += work_mem_h[i*(K+1)+K];
    }

    /* if opening a center at x saves cost (i.e. cost is negative) do so
       otherwise, do nothing */
    if ( gl_cost < 0 ) {
      for(int i=0; i<num; i++){
        bool close_center = gl_lower[center_table[points->p[i].assign]] > 0 ;
        if ( (switch_membership[i]=='1') || close_center ) {
          points->p[i].cost = points->p[i].weight * dist(points->p[i], points->p[x], points->dim);
          points->p[i].assign = x;
        }
      }

      for(int i=0; i<num; i++){
        if( is_center[i] && gl_lower[center_table[i]] > 0 )
          is_center[i] = false;
      }

      is_center[x] = true;
      *numcenters = *numcenters +1 - numclose;
    }
    else
      gl_cost = 0;

#ifdef PROFILE_TMP
    double t12 = gettime();
    *serial += t12 - t11;
#endif
    c++;
  }
  catch(string msg){
    printf("--cambine:%s\n", msg.c_str());
    exit(-1);    
  }
  catch(...){
    printf("--cambine: unknow reasons in pgain\n");
  }

#ifdef DEBUG
  FILE *fp = fopen("data_debug.txt", "a");
  fprintf(fp,"%d, %f\n", c, gl_cost);
  fclose(fp);
#endif
  return -gl_cost;
}
