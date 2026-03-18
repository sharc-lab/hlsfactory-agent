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
// kernel_baseToNumber

// 1 base use 2 bit, drop gap
// kernel_compressedData








// updateRepresentative

// kernel_makeTable

// kernel_cleanTable


// kernel_filter

// kernel_align



// --- from main.cu ---
#include <chrono>



// --- from utils.cu ---

//--------------------function--------------------//
// printUsage

// checkOption

// compare

// readFile


// --- from utils.h ---
#pragma once
#include <iostream> // cout
#include <fstream>  // ifstream
#include <vector>  // vector
#include <cstring>  // memcpy
#include <algorithm>  // sort
#include <hip/hip_runtime.h>
//--------------------data--------------------//
struct Option {
  std::string inputFile;
  std::string outputFile;
  float threshold;
  int wordLength;
};

struct Read {
  std::string data;
  std::string name;
};

//--------------------function--------------------//
void checkOption(int argc, char **argv, Option &option);
bool readFile(std::vector<Read> &reads, Option &option);
