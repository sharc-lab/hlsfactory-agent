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
#include <stdio.h>
#include <chrono>

// check the test result

// moving inversion tests with complementary patterns



// --- from kernels.h ---
#define TYPE unsigned long
#define MAX_ERR_RECORD_COUNT 10
#define BLOCKSIZE (1024*1024)

#define RECORD_ERR(err, p, expect, current) do{ \
    unsigned int idx = (*err += 1u); \
    idx = idx % MAX_ERR_RECORD_COUNT; \
    err_addr[idx] = (unsigned long)p; \
    err_expect[idx] = (unsigned long)expect; \
    err_current[idx] = (unsigned long)current; \
    err_second_read[idx] = (unsigned long)(*p); \
  } while(0)

//each thread is responsible for 1 BLOCKSIZE each time
void kernel0_write(char* ptr, unsigned long size)
{
  int idx = _bid_x * BLOCK_DIM_X + _tid_x;
  unsigned long n = size/BLOCKSIZE;
  int total_num_threads = GRID_DIM_X * BLOCK_DIM_X;

  for (int i = idx; i < n; i += total_num_threads) {
    unsigned long * start_p = (unsigned long*)(ptr + i*BLOCKSIZE);
    unsigned long * end_p = (unsigned long*)(ptr + (i+1)*BLOCKSIZE);
    unsigned long * p = start_p;
    unsigned int pattern = 1;
    unsigned int mask = 8;

    *p = pattern;
    pattern = (pattern << 1);
    while(p < end_p){
      p = (unsigned long*) (((unsigned long)start_p)|mask);

      if(p == start_p) {
        mask = (mask << 1);
        if (mask == 0) break;
        continue;
      }

      if (p >= end_p) break;

      *p = pattern;
      pattern = pattern <<1;
      mask = (mask << 1);
      if (mask == 0) break;
    }
  }
}

void kernel0_read(
    const char* ptr, unsigned long size,
    unsigned int* err_count,
    unsigned long* err_addr,
    unsigned long* err_expect,
    unsigned long* err_current,
    unsigned long* err_second_read)
{
  int idx = _bid_x * BLOCK_DIM_X + _tid_x;
  unsigned long n = size/BLOCKSIZE;
  int total_num_threads = GRID_DIM_X * BLOCK_DIM_X;

  for (int i = idx; i < n; i += total_num_threads) {
    unsigned long * start_p= (unsigned long*)(ptr + i*BLOCKSIZE);
    unsigned long * end_p = (unsigned long*)(ptr + (i+1)*BLOCKSIZE);
    unsigned long * p =start_p;
    unsigned int pattern = 1;
    unsigned int mask = 8;

    if (*p != pattern) RECORD_ERR(err_count, p, pattern, *p);

    pattern = (pattern << 1);
    while (p< end_p) {
      p = ( unsigned long*)( ((unsigned long)start_p)|mask);

      if(p == start_p) {
        mask = (mask << 1);
        if (mask == 0) break;
        continue;
      }

      if (p >= end_p) break;

      if (*p != pattern) RECORD_ERR(err_count, p, pattern, *p);

      pattern = pattern <<1;
      mask = (mask << 1);
      if (mask == 0) break;
    }
  }
}

void kernel1_write(char* ptr, unsigned long size)
{
  int idx = _bid_x * BLOCK_DIM_X + _tid_x;

  unsigned long* buf = ( unsigned long*)ptr;
  unsigned long n = size/sizeof(unsigned long);
  int total_num_threads = GRID_DIM_X * BLOCK_DIM_X;

  for (int i = idx; i < n; i += total_num_threads)
    buf[i] = (unsigned long)(buf+i);
}

void kernel1_read(
    const char* ptr, unsigned long size,
    unsigned int* err_count,
    unsigned long* err_addr,
    unsigned long* err_expect,
    unsigned long* err_current,
    unsigned long* err_second_read)
{
  unsigned long* buf = ( unsigned long*)ptr;
  int idx = _bid_x * BLOCK_DIM_X + _tid_x;
  unsigned long n = size/sizeof(unsigned long);
  int total_num_threads = GRID_DIM_X * BLOCK_DIM_X;

  for (int i = idx; i < n; i += total_num_threads) {
    if(buf[i] != (unsigned long)(buf+i))
      RECORD_ERR(err_count, &buf[i], (buf+i), buf[i]);
  }
}

void kernel_write(char* ptr, unsigned long size, TYPE p1)
{
  TYPE* buf = (TYPE*)ptr;
  int idx = _bid_x * BLOCK_DIM_X + _tid_x;
  unsigned long n = size/sizeof(TYPE);
  int total_num_threads = GRID_DIM_X * BLOCK_DIM_X;

  for (int i = idx; i < n; i+= total_num_threads)
    buf[i] = p1;
}

void kernel_read_write(
    char* ptr, unsigned long size, TYPE p1, TYPE p2,
    unsigned int* err_count,
    unsigned long* err_addr,
    unsigned long* err_expect,
    unsigned long* err_current,
    unsigned long* err_second_read)
{
  TYPE* buf = (TYPE*) ptr;
  int idx = _bid_x * BLOCK_DIM_X + _tid_x;
  unsigned long n =  size/sizeof(TYPE);
  int total_num_threads = GRID_DIM_X * BLOCK_DIM_X;
  TYPE localp;

  for (int i = idx; i < n; i += total_num_threads) {

    localp = buf[i];

    if (localp != p1) RECORD_ERR(err_count, &buf[i], p1, localp);

    buf[i] = p2;
  }
}

void kernel_read(
    const char* ptr, unsigned long size, TYPE p1,
    unsigned int* err_count,
    unsigned long* err_addr,
    unsigned long* err_expect,
    unsigned long* err_current,
    unsigned long* err_second_read)
{
  TYPE* buf = (TYPE*) ptr;
  int idx = _bid_x * BLOCK_DIM_X + _tid_x;
  unsigned long n =  size/sizeof(TYPE);
  int total_num_threads = GRID_DIM_X * BLOCK_DIM_X;
  TYPE localp;

  for (int i = idx; i < n; i += total_num_threads) {
    localp = buf[i];
    if (localp != p1) RECORD_ERR(err_count, &buf[i], p1, localp);
  }
}

void kernel5_init(char* ptr, unsigned long size)
{
  unsigned int * buf = (unsigned int*)ptr;
  int idx = _bid_x * BLOCK_DIM_X + _tid_x;
  unsigned long n = size/64;
  int total_num_threads = GRID_DIM_X * BLOCK_DIM_X;

  unsigned int p1 = 1;
  unsigned int p2;
  p1 = p1 << (idx % 32);
  p2 = ~p1;
  for (int i = idx; i < n; i+= total_num_threads){
    buf[i*16] = p1;
    buf[i*16 + 1] = p1;
    buf[i*16 + 2] = p2;
    buf[i*16 + 3] = p2;
    buf[i*16 + 4] = p1;
    buf[i*16 + 5] = p1;
    buf[i*16 + 6] = p2;
    buf[i*16 + 7] = p2;
    buf[i*16 + 8] = p1;
    buf[i*16 + 9] = p1;
    buf[i*16 + 10] = p2;
    buf[i*16 + 11] = p2;
    buf[i*16 + 12] = p1;
    buf[i*16 + 13] = p1;
    buf[i*16 + 14] = p2;
    buf[i*16 + 15] = p2;
  }
}

void kernel5_move(char* ptr, unsigned long size)
{
  int i, j;
  int idx = _bid_x * BLOCK_DIM_X + _tid_x;
  unsigned long n = size/BLOCKSIZE;
  int total_num_threads = GRID_DIM_X * BLOCK_DIM_X;
  unsigned int half_count = BLOCKSIZE/sizeof(unsigned int)/2;
  for (i = idx; i < n; i+= total_num_threads){
    unsigned int* mybuf = (unsigned int*)(ptr + i*BLOCKSIZE);
    unsigned int* mybuf_mid = (unsigned int*)(ptr + i*BLOCKSIZE + BLOCKSIZE/2);
    for (j = 0; j < half_count; j++)
      mybuf_mid[j] = mybuf[j];

    for (j = 0;j < half_count -8; j++)
      mybuf[j+8] = mybuf_mid[j];

    for (j = 0; j < 8; j++)
      mybuf[j] = mybuf_mid[half_count - 8+j];
  }
}

void kernel5_check(
    const char* ptr, unsigned long size,
    unsigned int* err_count,
    unsigned long* err_addr,
    unsigned long* err_expect,
    unsigned long* err_current,
    unsigned long* err_second_read)
{
  unsigned int * buf = (unsigned int*)ptr;
  int idx = _bid_x * BLOCK_DIM_X + _tid_x;
  unsigned long n = size/(2*sizeof(unsigned int));
  int total_num_threads = GRID_DIM_X * BLOCK_DIM_X;

  for (int i = idx; i < n; i += total_num_threads) {
    if (buf[2*i] != buf[2*i+1])
      RECORD_ERR(err_count, &buf[2*i], buf[2*i+1], buf[2*i]);
  }
}
