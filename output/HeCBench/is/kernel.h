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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <chrono>

/**********************/
/* partial verif info */
/**********************/
int test_index_array[TEST_ARRAY_SIZE],
    test_rank_array[TEST_ARRAY_SIZE],

    S_test_index_array[TEST_ARRAY_SIZE] = {48427,17148,23627,62548,4431},
    S_test_rank_array[TEST_ARRAY_SIZE] = {0,18,346,64917,65463},

    W_test_index_array[TEST_ARRAY_SIZE] = {357773,934767,875723,898999,404505},
    W_test_rank_array[TEST_ARRAY_SIZE] = {1249,11698,1039987,1043896,1048018},

    A_test_index_array[TEST_ARRAY_SIZE] = {2112377,662041,5336171,3642833,4250760},
    A_test_rank_array[TEST_ARRAY_SIZE] = {104,17523,123928,8288932,8388264},

    B_test_index_array[TEST_ARRAY_SIZE] = {41869,812306,5102857,18232239,26860214},
    B_test_rank_array[TEST_ARRAY_SIZE] = {33422937,10244,59149,33135281,99}, 

    C_test_index_array[TEST_ARRAY_SIZE] = {44172927,72999161,74326391,129606274,21736814},
    C_test_rank_array[TEST_ARRAY_SIZE] = {61147,882988,266290,133997595,133525895},

    D_test_index_array[TEST_ARRAY_SIZE] = {1317351170,995930646,1157283250,1503301535,1453734525},
    D_test_rank_array[TEST_ARRAY_SIZE] = {1,36538729,1978098519,2145192618,2147425337};

/* is */


// --- from is.h ---
/******************/
/* default values */
/******************/
#ifndef CLASS
#define CLASS 'S'
#endif

/*************/
/*  CLASS S  */
/*************/
#if CLASS == 'S'
#define TOTAL_KEYS_LOG_2 (16)
#define MAX_KEY_LOG_2 (11)
#define NUM_BUCKETS_LOG_2 (9)
#endif

/*************/
/*  CLASS W  */
/*************/
#if CLASS == 'W'
#define TOTAL_KEYS_LOG_2 (20)
#define MAX_KEY_LOG_2 (16)
#define NUM_BUCKETS_LOG_2 (10)
#endif

/*************/
/*  CLASS A  */
/*************/
#if CLASS == 'A'
#define TOTAL_KEYS_LOG_2 (23)
#define MAX_KEY_LOG_2 (19)
#define NUM_BUCKETS_LOG_2 (10)
#endif

/*************/
/*  CLASS B  */
/*************/
#if CLASS == 'B'
#define TOTAL_KEYS_LOG_2 (25)
#define MAX_KEY_LOG_2 (21)
#define NUM_BUCKETS_LOG_2 (10)
#endif

/*************/
/*  CLASS C  */
/*************/
#if CLASS == 'C'
#define TOTAL_KEYS_LOG_2 (27)
#define MAX_KEY_LOG_2 (23)
#define NUM_BUCKETS_LOG_2 (10)
#endif

#define TOTAL_KEYS (1 << TOTAL_KEYS_LOG_2)
#define MAX_KEY (1 << MAX_KEY_LOG_2)

#define NUM_BUCKETS (1 << NUM_BUCKETS_LOG_2)
#define NUM_KEYS (TOTAL_KEYS)
#define SIZE_OF_BUFFERS (NUM_KEYS)

#define MAX_ITERATIONS (24)
#define TEST_ARRAY_SIZE (5)

/*************************************/
/* typedef: if necessary, change the */
/* size of int here by changing the  */
/* int type to, say, long            */
/*************************************/
#if CLASS == 'D'
/* #TODO */
/* typedef long INT_TYPE; */
#else
/* typedef int INT_TYPE; */
#endif



// --- from kernels.h ---
#define R23 pow(0.5, 23.0)
#define R46 (R23*R23)
#define T23 pow(2.0, 23.0)
#define T46 (T23*T23)

double randlc_device(double* X, const double* A)
{
  double T1, T2, T3, T4;
  double A1;
  double A2;
  double X1;
  double X2;
  double Z;
  int j;

  /*
   * --------------------------------------------------------------------
   * break A into two parts such that A = 2^23 * A1 + A2 and set X = N.
   * --------------------------------------------------------------------
   */
  T1 = R23 * *A;
  j  = T1;
  A1 = j;
  A2 = *A - T23 * A1;

  /*
   * --------------------------------------------------------------------
   * break X into two parts such that X = 2^23 * X1 + X2, compute
   * Z = A1 * X2 + A2 * X1  (mod 2^23), and then
   * X = 2^23 * Z + A2 * X2  (mod 2^46). 
   * --------------------------------------------------------------------
   */
  T1 = R23 * *X;
  j  = T1;
  X1 = j;
  X2 = *X - T23 * X1;
  T1 = A1 * X2 + A2 * X1;

  j  = R23 * T1;
  T2 = j;
  Z = T1 - T23 * T2;
  T3 = T23 * Z + A2 * X2;
  j  = R46 * T3;
  T4 = j;
  *X = T3 - T46 * T4;

  return(R46 * *X);
}

double find_my_seed_device(
    int kn,
    int np,
    long nn,
    double s,
    double a)
{
  double t1,t2;
  long mq,nq,kk,ik;

  if(kn==0) return s;

  mq = (nn/4 + np - 1) / np;
  nq = mq * 4 * kn;

  t1 = s;
  t2 = a;
  kk = nq;
  while(kk > 1){
    ik = kk / 2;
    if(2*ik==kk){
      (void)randlc_device(&t2, &t2);
      kk = ik;
    }else{
      (void)randlc_device(&t1, &t2);
      kk = kk - 1;
    }
  }
  (void)randlc_device(&t1, &t2);

  return(t1);
}

void create_seq_gpu_kernel(
    int* key_array,
    double seed,
    double a,
    int number_of_blocks,
    int amount_of_work)
{
  double x, s;
  double an = a;
  int i, k;
  int k1, k2;
  int myid, num_procs;
  int mq;

  myid = _bid_x*BLOCK_DIM_X+_tid_x;
  num_procs = amount_of_work;

  mq = (NUM_KEYS + num_procs - 1) / num_procs;
  k1 = mq * myid;
  k2 = k1 + mq;
  if(k2 > NUM_KEYS) k2 = NUM_KEYS;

  s = find_my_seed_device(myid, num_procs, (long)4*NUM_KEYS, seed, an);

  k = MAX_KEY/4;

  for(i=k1; i<k2; i++){
    x = randlc_device(&s, &an);
    x += randlc_device(&s, &an);
    x += randlc_device(&s, &an);
    x += randlc_device(&s, &an);  
    key_array[i] = k*x;
  }
}

void full_verify_gpu_kernel_1(
    const int* key_array,
    int* key_buff2,
    int number_of_blocks,
    int amount_of_work)
{
  int i = _bid_x*BLOCK_DIM_X+_tid_x;
  key_buff2[i] = key_array[i];
}

void full_verify_gpu_kernel_2(
    const int*  key_buff2,
    int*  key_buff_ptr_global,
    int*  key_array,
    int number_of_blocks,
    int amount_of_work)
{    
  int value = key_buff2[_bid_x*BLOCK_DIM_X+_tid_x];
  int index = (key_buff_ptr_global[value] += -1) - 1;
  key_array[index] = value;
}

void full_verify_gpu_kernel_3(
    const int* key_array,
    int* global_aux,
    int number_of_blocks,
    int amount_of_work)
{
  int shared_data[4096];

  int i = (_bid_x*BLOCK_DIM_X+_tid_x) + 1;

  if(i < NUM_KEYS){
    if(key_array[i-1] > key_array[i]) 
      shared_data[_tid_x]=1;
    else
      shared_data[_tid_x]=0;
  }else
    shared_data[_tid_x]=0;

  for(i=BLOCK_DIM_X/2; i>0; i>>=1) {
    if(_tid_x<i)
      shared_data[_tid_x] += shared_data[_tid_x+i];
  }

  if(_tid_x==0) global_aux[_bid_x]=shared_data[0];
}

void rank_gpu_kernel_1(
    int* key_array,
    int* partial_verify_vals,
    const int* test_index_array,
    int iteration,
    int number_of_blocks,
    int amount_of_work)
{
  key_array[iteration] = iteration;
  key_array[iteration+MAX_ITERATIONS] = MAX_KEY - iteration;
  /*
   * --------------------------------------------------------------------
   * determine where the partial verify test keys are, 
   * --------------------------------------------------------------------
   * load into top of array bucket_size  
   * --------------------------------------------------------------------
   */
#pragma unroll
  for(int i=0; i<TEST_ARRAY_SIZE; i++){
    partial_verify_vals[i] = key_array[test_index_array[i]];
  }
}

void rank_gpu_kernel_2(
    int* key_buff1,
    int number_of_blocks,
    int amount_of_work)
{
  key_buff1[_bid_x*BLOCK_DIM_X+_tid_x] = 0;
}

void rank_gpu_kernel_3(
    int* key_buff_ptr,
    const int* key_buff_ptr2,
    int number_of_blocks,
    int amount_of_work)
{
  /*
   * --------------------------------------------------------------------
   * in this section, the keys themselves are used as their 
   * own indexes to determine how many of each there are: their
   * individual population  
   * --------------------------------------------------------------------
   */
  atomicAdd(&key_buff_ptr[key_buff_ptr2[_bid_x*BLOCK_DIM_X+_tid_x]], 1);
}

void rank_gpu_kernel_4(
    const int* source,
    int* destiny,
    int* sum,
    int number_of_blocks,
    int amount_of_work)
{
  int shared_data[4096];

  shared_data[_tid_x] = 0;
  int position = BLOCK_DIM_X + _tid_x;

  int factor = MAX_KEY / number_of_blocks;
  int start = factor * _bid_x;
  int end = start + factor;

  for(int i=start; i<end; i+=BLOCK_DIM_X){
    shared_data[position] = source[i + _tid_x];

    for(uint offset=1; offset<BLOCK_DIM_X; offset<<=1){
      int t = shared_data[position] + shared_data[position - offset];
      shared_data[position] = t;
    }

    int prv_val = (i == start) ? 0 : destiny[i - 1];
    destiny[i + _tid_x] = shared_data[position] + prv_val;
  }
  if(_tid_x==0) sum[_bid_x] = destiny[end-1];
}

void rank_gpu_kernel_5(
    const int* source,
    int* destiny,
    int number_of_blocks,
    int amount_of_work)
{
  int shared_data[4096];

  shared_data[_tid_x] = 0;
  int position = BLOCK_DIM_X + _tid_x;
  shared_data[position] = source[_tid_x];

  for(uint offset=1; offset<BLOCK_DIM_X; offset<<=1) {
    int t = shared_data[position] + shared_data[position - offset];
    shared_data[position] = t;
  }

  destiny[_tid_x] = shared_data[position - 1];
}

void rank_gpu_kernel_6(
    const int* source,
    int* destiny,
    const int* offset,
    int number_of_blocks,
    int amount_of_work)
{
  int factor = MAX_KEY / number_of_blocks;
  int start = factor * _bid_x;
  int end = start + factor;
  int sum = offset[_bid_x];
  for(int i=start; i<end; i+=BLOCK_DIM_X)
    destiny[i + _tid_x] = source[i + _tid_x] + sum;
}

void rank_gpu_kernel_7(
    const int* partial_verify_vals,
    const int* key_buff_ptr,
    const int* test_rank_array,
    int* passed_verification_device,
    int iteration,
    int number_of_blocks,
    int amount_of_work)
{
  /*
   * --------------------------------------------------------------------
   * this is the partial verify test section 
   * observe that test_rank_array vals are
   * shifted differently for different cases
   * --------------------------------------------------------------------
   */
  int i, k;
  int passed_verification = 0;
  for(i=0; i<TEST_ARRAY_SIZE; i++){  
    /* test vals were put here on partial_verify_vals */                                           
    k = partial_verify_vals[i];          
    if(0<k && k<=NUM_KEYS-1){
      int key_rank = key_buff_ptr[k-1];
      switch(CLASS){
        case 'S':
          if(i<=2){
            if(key_rank == test_rank_array[i]+iteration)
              passed_verification++;
          }else{
            if(key_rank == test_rank_array[i]-iteration)
              passed_verification++;
          }
          break;
        case 'W':
          if(i<2){
            if(key_rank == test_rank_array[i]+(iteration-2))
              passed_verification++;
          }else{
            if(key_rank == test_rank_array[i]-iteration)
              passed_verification++;
          }
          break;
        case 'A':
          if(i<=2){
            if(key_rank == test_rank_array[i]+(iteration-1))
              passed_verification++;
          }else{
            if(key_rank == test_rank_array[i]-(iteration-1))
              passed_verification++;
          }
          break;
        case 'B':
          if(i==1 || i==2 || i==4){
            if(key_rank == test_rank_array[i]+iteration)
              passed_verification++;
          }
          else{
            if(key_rank == test_rank_array[i]-iteration)
              passed_verification++;
          }
          break;
        case 'C':
          if(i<=2){
            if(key_rank == test_rank_array[i]+iteration)
              passed_verification++;
          }else{
            if(key_rank == test_rank_array[i]-iteration)
              passed_verification++;
          }
          break;
        case 'D':
          if(i<2){
            if(key_rank == test_rank_array[i]+iteration)
              passed_verification++;
          }else{
            if(key_rank == test_rank_array[i]-iteration)
              passed_verification++;
          }
          break;
      }
    }
  }
  *passed_verification_device += passed_verification;
}

