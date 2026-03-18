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

// --- from bwt.cu ---
#include <iostream>
#include <list>
#include "bwt.hpp"

const int blockSize = 256;





/* 
   returns a std::pair object
   the first item is the burrows wheeler transform of the input sequence in a std::string,
   the second item is the suffix array of the input sequence, represented as indicies of the given suffix, as an int*

   assumes input sequence already has ETX appended to it.
 */

std::pair<std::string,int*> bwt_with_suffix_array(const std::string sequence) {

  const int n = sequence.size();
  int table_size = sequence.size();
  // round the table size up to a power of 2 for bitonic sort
  table_size--;
  table_size |= table_size >> 1;
  table_size |= table_size >> 2;
  table_size |= table_size >> 4;
  table_size |= table_size >> 8;
  table_size |= table_size >> 16;
  table_size++;

  const int table_size_bytes = table_size * sizeof(int);
  const int seq_size_bytes = n * sizeof(char);

  int* d_table;

  int* table = (int*) malloc(table_size_bytes);

  int numBlocks = (table_size + blockSize - 1) / blockSize;

  char* d_sequence;


  char* d_transformed_sequence;

  numBlocks = (n + blockSize - 1) / blockSize;

  char* transformed_sequence_cstr = (char*) malloc(seq_size_bytes);

  std::string transformed_sequence(transformed_sequence_cstr, n);

  free(transformed_sequence_cstr);

  return std::make_pair(transformed_sequence, table);
}

