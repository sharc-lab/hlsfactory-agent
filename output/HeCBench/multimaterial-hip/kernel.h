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
#ifndef BLOCK_DIM_Y
#define BLOCK_DIM_Y 1
#endif
#ifndef GRID_DIM_Y
#define GRID_DIM_Y 1
#endif

// --- from compact.cu ---
#include <chrono>
#include <math.h>
#include <stdio.h>
#include <hip/hip_runtime.h>

char *cp_to_device(char *from, size_t size) {
  char *tmp;
  hipMalloc((void**)&tmp, size);
  hipMemcpy(tmp, from, size, hipMemcpyHostToDevice);
  return tmp;
}







// --- from full_matrix.cu ---
#include <math.h>
#include <stdio.h>

struct full_data
{
  int sizex;
  int sizey;
  int Nmats;
  double *  rho;
  double *  rho_mat_ave;
  double *  p;
  double *  Vf;
  double *  t;
  double *  V;
  double *  x;
  double *  y;
  double *  n;
  double *  rho_ave;
};





// --- from multimat.cu ---
/*
 * Open source copyright declaration based on BSD open source template:
 * http://www.opensource.org/licenses/bsd-license.php
 *
 * Copyright (c) 2013, Istvan Reguly and others. 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * The name of Mike Giles may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Istvan Reguly ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Mike Giles BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @brief initial version of mutli-material code with full dense matrix representaiton
 * @author Istvan Reguly
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <chrono>
#ifdef KNL
#include <hbwmalloc.h>
#else
#define hbw_malloc malloc
#define hbw_free free
#endif

struct full_data
{
  int sizex;
  int sizey;
  int Nmats;
  double *  rho;
  double *  rho_mat_ave;
  double *  p;
  double *  Vf;
  double *  t;
  double *  V;
  double *  x;
  double *  y;
  double *  n;
  double *  rho_ave;
};

struct compact_data
{
  int sizex;
  int sizey;
  int Nmats;
  double *  rho_compact;
  double *  rho_compact_list;
  double *  rho_mat_ave_compact;
  double *  rho_mat_ave_compact_list;
  double *  p_compact;
  double *  p_compact_list;
  double *  Vf_compact_list;
  double *  t_compact;
  double *  t_compact_list;
  double *  V;
  double *  x;
  double *  y;
  double *  n;
  double *  rho_ave_compact;
  int *  imaterial;
  int *  matids;
  int *  nextfrac;
  int *  mmc_index;
  int *  mmc_i;
  int *  mmc_j;
  int mm_len;
  int mmc_cells;
};

extern void full_matrix_cell_centric(full_data cc);

extern void full_matrix_material_centric(full_data cc, full_data mc);

extern bool full_matrix_check_results(full_data cc, full_data mc);

extern void compact_cell_centric(full_data cc, compact_data ccc, int argc, char** argv);




