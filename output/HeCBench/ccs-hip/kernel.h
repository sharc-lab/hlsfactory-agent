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
/*
 Condition-dependent Correlation Subgroups (CCS) 
 Description: Biclustering has been emerged as a powerful tool for 
 identification of a group of co-expressed genes under a subset 
 of experimental conditions (measurements) present in a gene 
 expression dataset.  In this program we implemented CCS biclustering. 

 Developer: Dr. Anindya Bhattacharya and Dr. Yan Cui, UTHSC, Memphis, TN, USA
 Email: anindyamail123@gmail.com; ycui2@uthsc.edu 

 Note: The minimum number of genes and the samples per bicluster is 10. 
 User can alter the minimum size by changing the values for 'mingene' 
 and 'minsample' defined in "ccs.h" file for minimum number of genes and samples
 respectively. 
*/

#include <chrono>
#include <hip/hip_runtime.h>
#include "ccs.h"
#include "matrixsize.c"
#include "readgene.c"
#include "pair_cor.c"
#include "bicluster_pair_score.c"
#include "merge_bicluster.c"
#include "print_bicluster.c"

// number of samples in the input datamatrix. 
// Fixed here to make static shared memory on a device
#define MAXSAMPLE 200 



