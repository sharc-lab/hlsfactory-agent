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

// --- from kernels.cu ---
#include <vector>
#include <iostream>
#include <chrono>

#define DIAMETER_SAMPLES 512

// This will output the proper CUDA error strings in the event that a CUDA host call returns an error
#ifndef checkCudaErrors
#define checkCudaErrors(err)  __checkCudaErrors (err, __FILE__, __LINE__)

// These are the inline versions for all of the SDK helper functions
#endif

//Note: N must be a power of two
//Simple/Naive bitonic sort. We're only sorting ~512 elements one time, so performance isn't important

void bc_kernel(
  float * bc,
  const int * R,
  const int * C,
  const int * F,
  const int n,
  const int m,
  const int * d,
  const unsigned long long * sigma,
  const float * delta,
  const int * Q,
  const int * Q2,
  const int * S,
  const int * endpoints,
  int * next_source,
  const size_t pitch_d,
  const size_t pitch_sigma,
  const size_t pitch_delta,
  const size_t pitch_Q,
  const size_t pitch_Q2,
  const size_t pitch_S,
  const size_t pitch_endpoints,
  const int start,
  const int end,
  int * jia,
  int * diameters,
  const int * source_vertices,
  const bool approx)
{
  int ind;
  int i;
  int *Q_row;
  int *Q2_row;
  int *S_row;
  int *endpoints_row;

  int j = _tid_x;
  int *d_row = (int*)((char*)d + _bid_x*pitch_d);
  unsigned long long *sigma_row = (unsigned long long*)((char*)sigma + _bid_x*pitch_sigma);
  float *delta_row = (float*)((char*)delta + _bid_x*pitch_delta);


}

std::vector<float> bc_gpu(
  graph g,
  int max_threads_per_block,
  int number_of_SMs,
  program_options op,
  const std::set<int> &source_vertices)
{
  float *bc_gpu = new float[g.n];
  int next_source = number_of_SMs; 

  float *bc_d, *delta_d;
  int *d_d, *R_d, *C_d, *F_d, *Q_d, *Q2_d, *S_d, *endpoints_d, *next_source_d, *source_vertices_d;
  unsigned long long *sigma_d;
  size_t pitch_d, pitch_sigma, pitch_delta, pitch_Q, pitch_Q2, pitch_S, pitch_endpoints;
  int *jia_d, *diameters_d;

  dim3 dimGrid (number_of_SMs, 1, 1);
  dim3 dimBlock (max_threads_per_block, 1, 1); 

  //Allocate and transfer data to the GPU

  checkCudaErrors(cudaMallocPitch((void**)&d_d,&pitch_d,sizeof(int)*g.n,dimGrid.x));
  checkCudaErrors(cudaMallocPitch((void**)&sigma_d,&pitch_sigma,sizeof(unsigned long long)*g.n,dimGrid.x));
  checkCudaErrors(cudaMallocPitch((void**)&delta_d,&pitch_delta,sizeof(float)*g.n,dimGrid.x));
  //Making Queues/Stack of size O(n) since we won't duplicate
  checkCudaErrors(cudaMallocPitch((void**)&Q_d,&pitch_Q,sizeof(int)*g.n,dimGrid.x));
  checkCudaErrors(cudaMallocPitch((void**)&Q2_d,&pitch_Q2,sizeof(int)*g.n,dimGrid.x));
  checkCudaErrors(cudaMallocPitch((void**)&S_d,&pitch_S,sizeof(int)*g.n,dimGrid.x));
  checkCudaErrors(cudaMallocPitch((void**)&endpoints_d,&pitch_endpoints,sizeof(int)*(g.n+1),dimGrid.x));

  // source_vertices of type "std::set" has no data() method
  std::vector<int> source_vertices_h(source_vertices.size());
  std::copy(source_vertices.begin(),source_vertices.end(),source_vertices_h.begin());

  

  int end;
  bool approx;

  auto start = std::chrono::steady_clock::now();
  

      bc_d,
      R_d,
      C_d,
      F_d,
      g.n,
      g.m,
      d_d,
      sigma_d,
      delta_d,
      Q_d,
      Q2_d,
      S_d,
      endpoints_d,
      next_source_d,
      pitch_d,
      pitch_sigma,
      pitch_delta,
      pitch_Q,
      pitch_Q2,
      pitch_S,
      pitch_endpoints,
      0,
      end,
      jia_d,
      diameters_d,
      source_vertices_d,
      approx);

  auto stop = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
  std::cout << "Kernel execution time " << time * 1e-9f << " (s)\n";

  // GPU result

  //Copy GPU result to a vector
  std::vector<float> bc_gpu_v(bc_gpu,bc_gpu+g.n);


  delete[] bc_gpu;
  return bc_gpu_v;
}

// query the properties of a single device for simplicity



// --- from parse.h ---
#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <set>
#include <vector>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <boost/bimap.hpp>

class graph 
{
  public:  
    graph() : R(NULL), C(NULL), F(NULL), n(-1), m(-1) {}

    void print_adjacency_list();
    void print_BC_scores(const std::vector<float> bc, char *outfile);
    void print_CSR();
    void print_R();
    void print_high_degree_vertices();
    void print_numerical_edge_file(char *outfile);
    void print_number_of_isolated_vertices();

    int *R;
    int *C;
    int *F;
    int n; //Number of vertices
    int m; //Number of edges
    boost::bimap<unsigned,std::string> IDs; 
    //Associate vertices with other data. In general the unsigned could be replaced with a struct of attributes. 
};

graph parse(char *file);
graph parse_metis(char *file);
graph parse_edgelist(char *file);


// --- from sequential.h ---
#ifndef SEQUENTIAL
#define SEQUENTIAL

#include <vector>
#include <set>
#include <queue>
#include <stack>

std::vector<float> bc_cpu(graph g, const std::set<int> &source_vertices);

#endif


// --- from util.h ---
#ifndef BC_UTIL
#define BC_UTIL

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <getopt.h>

//Command line parsing
class program_options
{
  public:
    program_options() : infile(NULL), verify(false), printBCscores(false), 
                        scorefile(NULL), device(-1), approx(false), k(256) {}

    char *infile;
    bool verify;
    bool printBCscores;
    char *scorefile;
    int device;
    bool approx;
    int k;
};

program_options parse_arguments(int argc, char *argv[]);

void query_device(int &max_threads_per_block, int &number_of_SMs, program_options op);

// compare cpu and gpu results
void verify(graph g, const std::vector<float> bc_cpu, const std::vector<float> bc_gpu);

// run bc on a GPU device
std::vector<float> bc_gpu(
  graph g,
  int max_threads_per_block,
  int number_of_SMs,
  program_options op,
  const std::set<int> &source_vertices);

#endif

