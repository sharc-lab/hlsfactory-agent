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
   ECL-MIS code: ECL-MIS is a maximal independent set algorithm that is quite
   fast and produces relatively large sets. It operates on graphs stored in
   binary CSR format.

   Copyright (c) 2017-2020, Texas State University. All rights reserved. Patented.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted (subject to the limitations in the disclaimer
   below) provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of Texas State University nor the names of its
 contributors may be used to endorse or promote products derived from
 this software without specific prior written permission.

 NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL TEXAS STATE UNIVERSITY
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

Authors: Martin Burtscher and Sindhu Devale

URL: The latest version of this code is available at
https://userweb.cs.txstate.edu/~burtscher/research/ECL-MIS/.

Publication: This work is described in detail in the following paper.
Martin Burtscher, Sindhu Devale, Sahar Azimi, Jayadharini Jaiganesh, and
Evan Powers. A High-Quality and Fast Maximal Independent Set Implementation
for GPUs. ACM Transactions on Parallel Computing, Vol. 5, No. 2, Article 8
(27 pages). December 2018.
 */

#include <stdlib.h>
#include <stdio.h>
#include <chrono>

static const int ThreadsPerBlock = 256;

typedef unsigned char stattype;
static const stattype in = 0xfe;
static const stattype out = 0;

/* main computation kernel */


/* hash function to generate random values */

// source of hash function: https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key

/* prioritized-selection initialization kernel */



    fprintf(stderr, "ERROR: could not allocate nlist_d\n\n");
  }

    fprintf(stderr, "ERROR: could not allocate nstat_d\n\n");
  }

    fprintf(stderr, "ERROR: copying to device failed\n\n");
  }

    fprintf(stderr, "ERROR: copying to device failed\n\n");
  }

  const int blocks = 24;

  auto start = std::chrono::high_resolution_clock::now();


  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  float runtime = (float)elapsed_seconds.count() / repeat;
  printf("compute time: %.6f s\n", runtime);
  printf("throughput: %.6f Mnodes/s\n", nodes * 0.000001 / runtime);
  printf("throughput: %.6f Medges/s\n", edges * 0.000001 / runtime);

  printf("ECL-MIS v1.3 (%s)\n", __FILE__);

  ECLgraph g = readECLgraph(argv[1]);
  printf("configuration: %d nodes and %d edges (%s)\n", g.nodes, g.edges, argv[1]);
  printf("average degree: %.2f edges per node\n", 1.0 * g.edges / g.nodes);

  stattype* nstatus = (stattype*)malloc(g.nodes * sizeof(nstatus[0]));

  else {
    const int repeat = atoi(argv[2]);

  }

  freeECLgraph(g);
  if (nstatus != NULL) free(nstatus);
  return 0;
}


// --- from graph.h ---
/*
   Copyright (c) 2016-2020, Texas State University. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of Texas State University nor the names of its
 contributors may be used to endorse or promote products derived from
 this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL TEXAS STATE UNIVERSITY BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Author: Martin Burtscher
*/

#ifndef ECL_GRAPH
#define ECL_GRAPH

#include <cstdlib>
#include <cstdio>

struct ECLgraph {
  int nodes;
  int edges;
  int* nindex;
  int* nlist;
  int* eweight;
};

ECLgraph readECLgraph(const char* const fname)
{
  ECLgraph g;
  int cnt;

  FILE* f = fopen(fname, "rb");
  if (f == NULL) {fprintf(stderr, "ERROR: could not open file %s\n\n", fname);  exit(-1);}
  cnt = fread(&g.nodes, sizeof(g.nodes), 1, f);
  if (cnt != 1) {fprintf(stderr, "ERROR: failed to read nodes\n\n");  exit(-1);}
  cnt = fread(&g.edges, sizeof(g.edges), 1, f);
  if (cnt != 1) {fprintf(stderr, "ERROR: failed to read edges\n\n");  exit(-1);}
  if ((g.nodes < 1) || (g.edges < 0)) {
    fprintf(stderr, "ERROR: node or edge count too low\n\n");
    exit(-1);
  }

  g.nindex = (int*)malloc((g.nodes + 1) * sizeof(g.nindex[0]));
  g.nlist = (int*)malloc(g.edges * sizeof(g.nlist[0]));
  g.eweight = (int*)malloc(g.edges * sizeof(g.eweight[0]));
  if ((g.nindex == NULL) || (g.nlist == NULL) || (g.eweight == NULL)) {
    fprintf(stderr, "ERROR: memory allocation failed\n\n");
    if (g.nindex != NULL) free(g.nindex);
    if (g.nlist != NULL) free(g.nlist);
    if (g.eweight != NULL) free(g.eweight);
    exit(-1);
  }

  cnt = fread(g.nindex, sizeof(g.nindex[0]), g.nodes + 1, f);
  if (cnt != g.nodes + 1) 
    fprintf(stderr, "ERROR: failed to read neighbor index list\n\n");
  cnt = fread(g.nlist, sizeof(g.nlist[0]), g.edges, f);
  if (cnt != g.edges) 
    fprintf(stderr, "ERROR: failed to read neighbor list\n\n");
  cnt = fread(g.eweight, sizeof(g.eweight[0]), g.edges, f);
  if (cnt == 0) {
    free(g.eweight);
    g.eweight = NULL;
  } else {
    if (cnt != g.edges) 
      fprintf(stderr, "ERROR: failed to read edge weights\n\n");
  }
  fclose(f);

  return g;
}

void freeECLgraph(ECLgraph &g)
{
  if (g.nindex != NULL) free(g.nindex);
  if (g.nlist != NULL) free(g.nlist);
  if (g.eweight != NULL) free(g.eweight);
  g.nindex = NULL;
  g.nlist = NULL;
  g.eweight = NULL;
}

#endif
