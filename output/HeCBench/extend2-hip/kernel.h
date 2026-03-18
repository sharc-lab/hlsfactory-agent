#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
using std::min;
using std::max;

// --- from main.cu ---
/* The MIT License
   Copyright (c) 2011 by Attractive Chaos <attractor@live.co.uk>
   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:
   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <chrono>
#include <hip/hip_runtime.h>
#include "read_data.h"


typedef struct {
  int h, e;
} eh_t;

void kernel_extend2(
    const unsigned char* query,
    const unsigned char* target,
    const char* mat,
    eh_t* eh,
    char* qp,
    int* qle_acc,
    int* tle_acc,
    int* gtle_acc,
    int* gscore_acc,
    int* max_off_acc,
    int* score_acc,
    const int qlen, 
    const int tlen, 
    const int m, 
    const int o_del, 
    const int e_del, 
    const int o_ins, 
    const int e_ins, 
    int w, 
    const int end_bonus, 
    const int zdrop, 
    const int h0)
{
  int oe_del = o_del + e_del;
  int oe_ins = o_ins + e_ins; 
  int i, j, k;
  int beg, end;
  int max, max_i, max_j, max_ins, max_del, max_ie;
  int gscore;
  int max_off;

  // generate the query profile

  // fill the first row
  eh[0].h = h0; 
  eh[1].h = h0 > oe_ins? h0 - oe_ins : 0;

  for (j = 2; j <= qlen && eh[j-1].h > e_ins; ++j)
    eh[j].h = eh[j-1].h - e_ins;

  // adjust $w if it is too large
  k = m * m;
  for (i = 0, max = 0; i < k; ++i) // get the max score
    max = max > mat[i]? max : mat[i];
  max_ins = (int)((float)(qlen * max + end_bonus - o_ins) / e_ins + 1.f);
  max_ins = max_ins > 1? max_ins : 1;
  w = w < max_ins? w : max_ins;
  max_del = (int)((float)(qlen * max + end_bonus - o_del) / e_del + 1.f);
  max_del = max_del > 1? max_del : 1;
  w = w < max_del? w : max_del; // TODO: is this necessary?
  // DP loop
  max = h0, max_i = max_j = -1; max_ie = -1, gscore = -1;
  max_off = 0;
  beg = 0, end = qlen;
  *qle_acc = max_j + 1;
  *tle_acc = max_i + 1;
  *gtle_acc = max_ie + 1;
  *gscore_acc = gscore;
  *max_off_acc = max_off;
  *score_acc = max;
}


