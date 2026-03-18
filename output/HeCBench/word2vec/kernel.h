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

// --- from cbow.cu ---
#include <stdio.h>
#include <assert.h>

real expTable[EXP_TABLE_SIZE];

extern real *syn0;
extern int * table;
extern int vocab_size, layer1_size , layer1_size_aligned;
extern int negative , window;
extern int table_size;
// To batch data to minimize data transfer, sen stores words + alpha values
// alpha value start at offset = MAX_SENTENCE_NUM * MAX_SENTENCE_LENGTH

extern int * sen;

real * d_syn0 = NULL;
real * d_syn1neg = NULL;
int  * d_sen = NULL;
unsigned int * d_random = NULL;
int * d_table = NULL;

int maxThreadsPerBlock = 256;
int numBlock;
int shared_mem_usage;




#define cudaCheck(err) { \
}





// --- from word2vec.cu ---
//  Copyright 2013 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <chrono>

const int vocab_hash_size = 30000000;  // Maximum 30 * 0.7 = 21M words in the vocabulary

// Precision of float numbers

struct vocab_word {
  int cn;
  int *point;
  char *word, *code, codelen;
};

char train_file[MAX_STRING], output_file[MAX_STRING];
char save_vocab_file[MAX_STRING], read_vocab_file[MAX_STRING];
struct vocab_word *vocab;
int binary = 0, cbow = 1, debug_mode = 2, window = 5, min_count = 5, num_threads = 12, min_reduce = 1;
int *vocab_hash;
int vocab_max_size = 1000, vocab_size = 0, layer1_size = 100,  layer1_size_aligned;;
long long train_words = 0, word_count_actual = 0, file_size = 0;
int iter = 5,  classes = 0;
real alpha = 0.025, starting_alpha, sample = 1e-3;
real *syn0;
int * sen;
auto start = std::chrono::steady_clock::now();

int hs = 0, negative = 5;
int table_size = 1e8;
int *table;


// Reads a single word from a file, assuming space + tab + EOL to be word boundaries

// Returns hash value of a word

// Returns position of a word in the vocabulary; if the word is not found, returns -1

// Reads a word and returns its index in the vocabulary

// Adds a word to the vocabulary

// Used later for sorting by word counts

// Sorts the vocabulary by frequency using word counts

// Reduces the vocabulary by removing infrequent tokens

// Create binary Huffman tree using the word counts
// Frequent words will have short uniqe binary codes





void *TrainModelThread(void *id) {
  int   word, sentence_length = 0;
  long long word_count = 0, last_word_count = 0;
  int    local_iter = iter;
  unsigned int next_random = (long)id;
  int sentence_num;
  real * alpha_ptr = (float *) sen + MAX_SENTENCE_NUM * MAX_SENTENCE_LENGTH;
  FILE *fi = fopen(train_file, "rb");
  fseek(fi, file_size / (int)num_threads * (long)id, SEEK_SET);
  sentence_length = 0;
  sentence_num = 0;


  GetResultFromGPU();
  fclose(fi);

  // Check output file first
  if (output_file[0] == 0) return;
  FILE *fo = fopen(output_file, "wb");

  long a, b, c, d;
  pthread_t *pt = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
  printf("Starting training using file %s\n", train_file);
  starting_alpha = alpha;
  if (read_vocab_file[0] != 0) ReadVocab(); else LearnVocabFromTrainFile();
  if (save_vocab_file[0] != 0) SaveVocab();

  InitNet();

  if (negative > 0) InitUnigramTable();

  initializeGPU();

  start = std::chrono::steady_clock::now();

  // Training on a GPU
  for (a = 0; a < num_threads; a++) pthread_create(&pt[a], NULL, TrainModelThread, (void *)a);

  // Training complete
  for (a = 0; a < num_threads; a++) pthread_join(pt[a], NULL);

  free(pt);
  int a;
  return -1;
}



// --- from cbow.h ---
/*
 * cbow.h
 *
 *  Created on: Aug 29, 2015
 *      Author: gpgpu
 */

#ifndef CBOW_H_
#define CBOW_H_

#define MAX_STRING 100
#define EXP_TABLE_SIZE 1000
#define MAX_EXP 6
#define MAX_SENTENCE_LENGTH 1024
#define MAX_CODE_LENGTH 40
#define MAX_SENTENCE_NUM 6
#define ALIGNMENT_FACTOR 32
#define THREADS_PER_WORD 128
#define BLOCK_SIZE 128
typedef float real;

void TrainGPU(int sentence_num);
void GetResultFromGPU();
void initializeGPU();
void cleanUpGPU();

#endif /* CBOW_H_ */
