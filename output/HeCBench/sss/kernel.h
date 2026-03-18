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
#define KERNELS


// shared myInt demand: p+2*nTreeEdges+BLOCK_SIZE


// --- from main.cu ---
// Chiranjit Mukherjee  (chiranjit@soe.ucsc.edu)

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <chrono>

#define SSS     // Runs the Stochastic Shotgun Search
#define USE_GPU // If uncommented, runs kernels on GPU
#define MAXL 10 // Maximum number of mixture components that can be accommodated

#ifdef SSS
// SSS- runtime parameters
// #define maxLocalWastedIterations (n+p)  // In paper, C
#define maxLocalWastedIterations 50

#define climbDownStepSize 10               // In paper, D

// #define maxLocalJumpCount 10            // In paper, R
#define maxLocalJumpCount 3

#define MAXNGLOBALJUMP 2                   // In paper, S / (C * R)
// SSS- parameters for lists of models saved

#define sizeOfFeatureSelectionList 20      // In paper, M

#define sizeOfBestList 100 // Number of highest-score models to keep track of
// SSS-
#define LOCALMOVE_SFACTOR 0.001
#define GLOBALJUMP_SFACTOR 0.01
#define G_TO_XI int(L * p * (p - 1) / (2 * n)) // In paper, g
#define XI_TO_SM 10                            // In paper, h
#define LOOKFORWARD 5                          // In paper, f
#define RGMS_T 2                               // In paper, t
// number of chains parameters
#define N_INIT                                                                 \
  1 // Number of points of initial models provided by the user in folder DATA/

#define TRY_EACH_INIT                                                          \
  1 // Number of times to restart from each given initial point

#define N_RANDOM_RESTART                                                       \
  1 // Number of times to restart from random random initial points

#define N_MODES_LIST_RESTART 1 // Number of times to start from

// #define maxNmodes
// ((TRY_EACH_INIT*N_INIT+N_RANDOM_RESTART+N_MODES_LIST_RESTART)+1)
#define maxNmodes 2
#endif

#define PI 3.1415926
#define log_2 0.693147180559945
#define log_pi_over_4 0.286182471462350
#define log_2_pi 1.837877066409345
#define NEG_INF -999999.0
#define myBool bool
#define myInt short // Using short interger
#define myIntFactor 2
#define intFactor 4
// #define Real double
#define Real float // Using floating-point
#define ISFLOAT 10
using namespace std;

#include <gsl/gsl_integration.h>
#include <gsl/gsl_sf.h>
#define GSL_INTEGRATION_GRIDSIZE 1000
gsl_integration_workspace *w;
gsl_function F;

#include <gsl/gsl_randist.h>
#define RANDOMSEED 314159265

// Define hyperparameters for the prior distribution of (mu, K | G)
#define N0 0.01
#define DELTA0 3
#define JEFFREYS_PRIOR
gsl_rng *rnd;

#ifdef USE_GPU
#define BLOCK_SIZE 32
#define SYNC __syncthreads()
typedef struct {
  cudaStream_t delete_stream;
  cudaStream_t add_stream;
  myInt *d_in_delete;
  myInt *d_in_add;
  myInt *d_which_delete;
  myInt *d_which_add;
  myInt *h_in_delete;
  myInt *h_in_add;
  myInt *which_delete;
  myInt *which_add;
  int n_add, n_delete;
} MGPUstuff;
#else
typedef struct {
} MGPUstuff;
#endif
MGPUstuff device;

// Include source files
#include "utilities.cpp"
#ifndef GRAPH_CPP
#include "graph.cpp"
#endif
#ifndef GWISH_CPP
#include "gwish.cpp"
#endif
#ifndef DPMIXGGM_CPP
#include "DPmixGGM.cpp"
#endif
#ifndef LISTS_CPP
#include "DPmixGGM_Lists.cpp"
#endif
#ifndef SSSMOVES_CPP
#include "DPmixGGM_SSSmoves.cpp"
#endif

//////////////////////////////////////////////////////////////// START OF MAIN
//////////////////////////////////////////////////////////////////



// --- from graph.h ---
#define GRAPH_H

typedef class Graph *LPGraph;

class Graph {
public:
  // data members
  myInt nVertices;
  myInt *d_nVertices; // number of vertices in the graph
  myInt **Edge;
  int *d_EdgeField; // matrix containing the edges of the graph

  myInt *Labels;
  myInt *d_Labels; // identifies the connected components of the graph
  myInt nLabels;
  myInt *d_nLabels;     // number of labels or connected components
  myInt **Cliques;      // storage for cliques
  myInt *CliquesDimens; // number of vertices in each clique
  myInt nCliques;
  myInt *d_nCliques; // number of cliques

public:
  myInt *TreeEdgeA; // edges of the clique tree
  myInt *TreeEdgeB;
  myInt nTreeEdges; // number of edges in the generated clique tree
public:
  myInt **Separators; // storage for separators
  myInt *SeparatorsDimens;
  myInt nSeparators;
  // private:
  myInt *localord;

  // methods
public:
  Graph();                     // constructor
  Graph(LPGraph InitialGraph); // constructor
  ~Graph();                    // destructor
public:
  myInt SearchVertex();       // identifies the next vertex to be eliminated
  void FlipEdge(myInt which); // flips an edge on a graph which is a myInteger
                              // between 0 and the total number of edges

public:
  // the MSS (Minimal Sufficient Statistics) are the maximal cliques for our
  // graph
  void InitGraph(myInt n);
  void CopyGraph(LPGraph G);
  void GenerateCliques(myInt label);
  myInt CheckCliques(myInt start,
                     myInt end); // checks whether each generated component is
                                 // complete in the given graph
  myInt IsClique(
      myInt *vect,
      myInt nvect); // checks if the vertices in vect form a clique in our graph

  void GenerateSeparators();
  void AttachLabel(myInt v, myInt label);
  void GenerateLabels();
  myInt GenerateAllCliques();
  myInt IsDecomposable();
  myInt IfDecomposable();

  myInt CanDeleteEdge(myInt a, myInt b);
  bool CanAddEdge(myInt a, myInt b);

  Real ScoreDeleteEdge(myInt a, myInt b, myInt which_ab, Real *D_prior,
                       Real *D_post, myInt delta, myInt n_sub, Real score,
                       int nEdges);
  Real ScoreAddEdge(myInt a, myInt b, Real *D_prior, Real *D_post, myInt delta,
                    myInt n_sub, Real score, int nEdges);
};

//////////////////////////////////////////////////////////////////////

typedef class SectionGraph *LPSectionGraph;

class SectionGraph : public Graph {
public:
  myInt *
      Eliminated; // shows which vertices were eliminated from the initial graph
  myInt nEliminated; // number of vertices we eliminated

  // methods
public:
  SectionGraph(LPGraph InitialGraph, myInt *velim); // constructor
  ~SectionGraph();                                  // destructor

public:
  myInt IsChain(myInt u, myInt v); // see if there is a chain between u and v
                                   // or, equivalently, checks if u and v are in
                                   // the same connected component
};

////////////////////////////////////////////////////////////////////////

typedef class EliminationGraph *LPEliminationGraph;

class EliminationGraph : public Graph {
public:
  myInt *
      Eliminated; // shows which vertices were eliminated from the initial graph
  myInt nEliminated; // number of vertices we eliminated

  // methods
public:
  EliminationGraph(LPGraph InitialGraph, myInt vertex); // constructor
  ~EliminationGraph();                                  // destructor
public:
  myInt SearchVertex(); // identify a vertex to be eliminated
public:
  void EliminateVertex(myInt x); // eliminates an extra vertex
};

//////////////////////////////////////////////////////////////////////////

// constructs the minimum fill-in graph for a nondecomposable graph
void TurnFillInGraph(LPGraph graph);