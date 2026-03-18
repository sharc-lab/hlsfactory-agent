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
 * The MIT License (MIT)
 *
 * Copyright (c) 2014, Erick Lavoie, Faiz Khan, Sujay Kathrotia, Vincent
 * Foley-Bourgon, Laurie Hendren
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 *of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <getopt.h>
#include <chrono>

#define D_FACTOR (0.85f)
#ifndef BLOCK_SIZE
#define BLOCK_SIZE 256
#endif

// default values 
const int max_iter = 1000;
const float threshold= 1e-16f;



// generates an array of random pages and their links
int *random_pages(int n, unsigned int *noutlinks, int divisor){
  int i, j, k;
  int *pages = (int*) malloc((size_t)n * n * sizeof(int)); // matrix 1 means link from j->i


  return pages;
}



static struct option size_opts[] =
{
  /* name, has_tag, flag, val*/
  {"number of pages", 1, NULL, 'n'},
  {"max number of iterations", 1, NULL, 'i'},
  {"minimum threshold", 1, NULL, 't'},
  {"divisor for zero density", 1, NULL, 'q'},
  { 0, 0, 0}
};


