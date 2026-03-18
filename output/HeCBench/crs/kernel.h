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

// --- from GCRSMatrix.cu ---
//
//  GCRSMatrix.c
//  NoSynFree
//
//  Created by Liu Chengjian on 17/8/29.
//  Copyright (c) 2017 csliu. All rights reserved.
//



int *gcrs_create_bitmatrix(int k, int m, int w){
  int i, j;
  int *matrix, *bitmatrix;


  matrix = talloc(int, k*m);



  bitmatrix = jerasure_matrix_to_bitmatrix(k, m, w, matrix);

  free(matrix);

  return bitmatrix;
}

unsigned int *gcrs_create_column_coding_bitmatrix(int k, int m, int w, int *bitmatrix){
  int columnIdx, rowIdx;
  int freeBitmatrixMark = 0, bitmatrixIdx;
  int intbitmatrixIdx = 0;
  int bitIdx = 0;
  unsigned int bitOne = 0x01;
  unsigned int *column_encoded_bitmatrix;



  //    int uIntSize = sizeof(unsigned int);
  //    int wUnitsPerUInt = uIntSize/w;
  //    int mwReqSize = m/wUnitsPerUInt;
  //    
  //    if (m%wUnitsPerUInt != 1) {
  //        mwReqSize = mwReqSize + 1;
  //    }

  column_encoded_bitmatrix = talloc(unsigned int, k * w * 2);




  return column_encoded_bitmatrix;
}



int *gcrs_create_decoding_bitmatrix(int k, int m, int w,
    int *matrix, int *mat_idx,
    int *erasures, int *dm_ids){
  int kIdx, mIdx, matIdx;
  int *erases;
  int *decoding_matrix, *decoding_data_matrix;
  int dFailedNum = 0;

  decoding_matrix = talloc(int, (k * w * w * m));






  matIdx = 0;


  free(erases);
  free(decoding_data_matrix);

  return decoding_matrix;
}

int *gcrs_erasures_to_erased(int k, int m, int *erasures)
{
  int td;
  int t_non_erased;
  int *erased;
  int i;

  td = k+m;
  erased = talloc(int, td);
  if (erased == NULL) return NULL;
  t_non_erased = td;

  return erased;
}






// --- from galois.cu ---
/* Galois.c
 * James S. Plank
 * April, 2007

Galois.tar - Fast Galois Field Arithmetic Library in C/C++
Copright (C) 2007 James S. Plank

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

James S. Plank
Department of Electrical Engineering and Computer Science
University of Tennessee
Knoxville, TN 37996
plank@cs.utk.edu

 */

/*
 * Part of jerasure: 
 * $Revision: 1.2 $
 * $Date: 2008/08/19 17:40:58 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define NONE (10)
#define TABLE (11)
#define SHIFT (12)
#define LOGS (13)
#define SPLITW8 (14)

static int prim_poly[33] = 
{ 0, 
/*  1 */     1, 
/*  2 */    07,
/*  3 */    013,
/*  4 */    023,
/*  5 */    045,
/*  6 */    0103,
/*  7 */    0211,
/*  8 */    0435,
/*  9 */    01021,
/* 10 */    02011,
/* 11 */    04005,
/* 12 */    010123,
/* 13 */    020033,
/* 14 */    042103,
/* 15 */    0100003,
/* 16 */    0210013,
/* 17 */    0400011,
/* 18 */    01000201,
/* 19 */    02000047,
/* 20 */    04000011,
/* 21 */    010000005,
/* 22 */    020000003,
/* 23 */    040000041,
/* 24 */    0100000207,
/* 25 */    0200000011,
/* 26 */    0400000107,
/* 27 */    01000000047,
/* 28 */    02000000011,
/* 29 */    04000000005,
/* 30 */    010040000007,
/* 31 */    static_cast<int>(020000000011), 
/* 32 */    00020000007 };  /* Really 40020000007, but we're omitting the high order bit */

static int mult_type[33] = 
{ NONE, 
/*  1 */   TABLE, 
/*  2 */   TABLE,
/*  3 */   TABLE,
/*  4 */   TABLE,
/*  5 */   TABLE,
/*  6 */   TABLE,
/*  7 */   TABLE,
/*  8 */   TABLE,
/*  9 */   TABLE,
/* 10 */   LOGS,
/* 11 */   LOGS,
/* 12 */   LOGS,
/* 13 */   LOGS,
/* 14 */   LOGS,
/* 15 */   LOGS,
/* 16 */   LOGS,
/* 17 */   LOGS,
/* 18 */   LOGS,
/* 19 */   LOGS,
/* 20 */   LOGS,
/* 21 */   LOGS,
/* 22 */   LOGS,
/* 23 */   SHIFT,
/* 24 */   SHIFT,
/* 25 */   SHIFT,
/* 26 */   SHIFT,
/* 27 */   SHIFT,
/* 28 */   SHIFT,
/* 29 */   SHIFT,
/* 30 */   SHIFT,
/* 31 */   SHIFT,
/* 32 */   SPLITW8 };

static int nw[33] = { 0, (1 << 1), (1 << 2), (1 << 3), (1 << 4), 
(1 << 5), (1 << 6), (1 << 7), (1 << 8), (1 << 9), (1 << 10),
(1 << 11), (1 << 12), (1 << 13), (1 << 14), (1 << 15), (1 << 16),
(1 << 17), (1 << 18), (1 << 19), (1 << 20), (1 << 21), (1 << 22),
(1 << 23), (1 << 24), (1 << 25), (1 << 26), (1 << 27), (1 << 28),
(1 << 29), (1 << 30), (1 << 31), -1 };

static int nwm1[33] = { 0, (1 << 1)-1, (1 << 2)-1, (1 << 3)-1, (1 << 4)-1, 
(1 << 5)-1, (1 << 6)-1, (1 << 7)-1, (1 << 8)-1, (1 << 9)-1, (1 << 10)-1,
(1 << 11)-1, (1 << 12)-1, (1 << 13)-1, (1 << 14)-1, (1 << 15)-1, (1 << 16)-1,
(1 << 17)-1, (1 << 18)-1, (1 << 19)-1, (1 << 20)-1, (1 << 21)-1, (1 << 22)-1,
(1 << 23)-1, (1 << 24)-1, (1 << 25)-1, (1 << 26)-1, (1 << 27)-1, (1 << 28)-1,
(1 << 29)-1, (1 << 30)-1, 0x7fffffff, static_cast<int>(0xffffffff) };
   
static int *galois_log_tables[33] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static int *galois_ilog_tables[33] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static int *galois_mult_tables[33] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static int *galois_div_tables[33] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

/* Special case for w = 32 */

static int *galois_split_w8[7] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };















/* This will destroy mat, by the way */




int *galois_get_mult_table(int w)
{
  return galois_mult_tables[w];
}

int *galois_get_div_table(int w) 
{
  return galois_div_tables[w];
}

int *galois_get_log_table(int w)
{
  return galois_log_tables[w];
}

int *galois_get_ilog_table(int w)
{
  return galois_ilog_tables[w];
}






// --- from jerasure.cu ---
/* jerasure.c
 * James S. Plank

Jerasure - A C/C++ Library for a Variety of Reed-Solomon and RAID-6 Erasure Coding Techniques
Copright (C) 2007 James S. Plank

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

James S. Plank
Department of Electrical Engineering and Computer Science
University of Tennessee
Knoxville, TN 37996
plank@cs.utk.edu
*/

/*
 * $Revision: 1.2 $
 * $Date: 2008/08/19 17:40:58 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define talloc(type, num) (type *) malloc(sizeof(type)*(num))

static double jerasure_total_xor_bytes = 0;
static double jerasure_total_gf_bytes = 0;
static double jerasure_total_memcpy_bytes = 0;




/* Internal Routine */


int *jerasure_matrix_to_bitmatrix(int k, int m, int w, int *matrix) 
{
  int *bitmatrix;
  int rowelts, rowindex, colindex, elt, i, j, l, x;

  bitmatrix = talloc(int, k*m*w*w);

  rowelts = k * w;
  rowindex = 0;

  return bitmatrix;
}






/* Converts a list-style version of the erasures into an array of k+m elements
   where the element = 1 if the index has been erased, and zero otherwise */

int *jerasure_erasures_to_erased(int k, int m, int *erasures)
{
  int td;
  int t_non_erased;
  int *erased;
  int i;

  td = k+m;
  erased = talloc(int, td);
  if (erased == NULL) return NULL;
  t_non_erased = td;

  return erased;
}
  




static char **set_up_ptrs_for_scheduled_decoding(int k, int m, int *erasures, char **data_ptrs, char **coding_ptrs)
{
  int *erased;
  char **ptrs;
  int i, j, x;

  erased = jerasure_erasures_to_erased(k, m, erasures);
  if (erased == NULL) return NULL;

  /* Set up ptrs.  It will be as follows:

       - If data drive i has not failed, then ptrs[i] = data_ptrs[i].
       - If data drive i has failed, then ptrs[i] = coding_ptrs[j], where j is the 
            lowest unused non-failed coding drive.
       - Elements k to k+ddf-1 are data_ptrs[] of the failed data drives.
       - Elements k+ddf to k+ddf+cdf-1 are coding_ptrs[] of the failed data drives.

       The array row_ids contains the ids of ptrs.
       The array ind_to_row_ids contains the row_id of drive i.
  
       However, we're going to set row_ids and ind_to_row in a different procedure.
   */
         
  ptrs = talloc(char *, k+m);

  j = k;
  x = k;
  free(erased);
  return ptrs;
}


static int **jerasure_generate_decoding_schedule(int k, int m, int w, int *bitmatrix, int *erasures, int smart)
{
  int i, j, x, drive, y, index, z;
  int *decoding_matrix, *inverse, *real_decoding_matrix;
  int *ptr;
  int *row_ids;
  int *ind_to_row;
  int ddf, cdf;
  int **schedule;
  int *b1, *b2;
 
 /* First, figure out the number of data drives that have failed, and the
    number of coding drives that have failed: ddf and cdf */

  ddf = 0;
  cdf = 0;
  
  row_ids = talloc(int, k+m);
  ind_to_row = talloc(int, k+m);

  if (set_up_ids_for_scheduled_decoding(k, m, erasures, row_ids, ind_to_row) < 0) return NULL;

  /* Now, we're going to create one decoding matrix which is going to 
     decode everything with one call.  The hope is that the scheduler
     will do a good job.    This matrix has w*e rows, where e is the
     number of erasures (ddf+cdf) */

  real_decoding_matrix = talloc(int, k*w*(cdf+ddf)*w);

  /* First, if any data drives have failed, then initialize the first
     ddf*w rows of the decoding matrix from the standard decoding
     matrix inversion */


  /* Next, here comes the hard part.  For each coding node that needs
     to be decoded, you start by putting its rows of the distribution
     matrix into the decoding matrix.  If there were no failed data
     nodes, then you're done.  However, if there have been failed
     data nodes, then you need to modify the columns that correspond
     to the data nodes.  You do that by first zeroing them.  Then
     whereever there is a one in the distribution matrix, you XOR
     in the corresponding row from the failed data node's entry in
     the decoding matrix.  The whole process kind of makes my head
     spin, but it works.
   */


/*
  printf("\n\nReal Decoding Matrix\n\n");
  jerasure_print_bitmatrix(real_decoding_matrix, (ddf+cdf)*w, k*w, w);
  free(row_ids);
  free(ind_to_row);
  free(real_decoding_matrix);
  return schedule;
}



/* This only works when m = 2 */

int ***jerasure_generate_schedule_cache(int k, int m, int w, int *bitmatrix, int smart)
{
  int ***scache;
  int erasures[3];
  int e1, e2;
 
  /* Ok -- this is yucky, but it's how I'm doing it.  You will make an index out
     of erasures, which will be  e1*(k+m)+(e2).  If there is no e2, then e2 = e1.
     Isn't that clever and confusing.  Sorry.

     We're not going to worry about ordering -- in other words, the schedule for
     e1,e2 will be the same as e2,e1.  They will have the same pointer -- the 
     schedule will not be duplicated. */

  if (m != 2) return NULL;

  scache = talloc(int **, (k+m)*(k+m+1));
  return scache;

}



  
int *jerasure_matrix_multiply(int *m1, int *m2, int r1, int c1, int r2, int c2, int w)
{
  int *product, i, j, k;

  product = (int *) malloc(sizeof(int)*r1*c2);
  return product;
}




    
int **jerasure_dumb_bitmatrix_to_schedule(int k, int m, int w, int *bitmatrix)
{
  int **operations;
  int op;
  int index, optodo, i, j;

  operations = talloc(int *, k*m*w*w+1);
  op = 0;
  
  index = 0;
  operations[op] = talloc(int, 5);
  operations[op][0] = -1;
  return operations;
}

int **jerasure_smart_bitmatrix_to_schedule(int k, int m, int w, int *bitmatrix)
{
  int **operations;
  int op;
  int i, j;
  int *diff, *from, *b1, *flink, *blink;
  int *ptr, no, row;
  int optodo;
  int bestrow, bestdiff, top;

/*   printf("Scheduling:\n\n");
  jerasure_print_bitmatrix(bitmatrix, m*w, k*w, w); */

  operations = talloc(int *, k*m*w*w+1);
  op = 0;
  
  diff = talloc(int, m*w);
  from = talloc(int, m*w);
  flink = talloc(int, m*w);
  blink = talloc(int, m*w);

  ptr = bitmatrix;

  bestdiff = k*w+1;
  top = 0;

  flink[m*w-1] = -1;
  
  
  operations[op] = talloc(int, 5);
  operations[op][0] = -1;
  free(from);
  free(diff);
  free(blink);
  free(flink);

  return operations;
}




// --- from kernels.cu ---








































void (*coding_func_array[])(int k, int index,
    char *dataPtr, char *codeDevPtr,
    const unsigned int *bitMatrixPtr, 
    int threadDimX,int blockDimX,
    int workSizePerGridInLong) = {
  m_1_w_4_coding,m_1_w_5_coding,m_1_w_6_coding,m_1_w_7_coding,m_1_w_8_coding,
  m_2_w_4_coding,m_2_w_5_coding,m_2_w_6_coding,m_2_w_7_coding,m_2_w_8_coding,
  m_3_w_4_coding,m_3_w_5_coding,m_3_w_6_coding,m_3_w_7_coding,m_3_w_8_coding,
  m_4_w_4_coding,m_4_w_5_coding,m_4_w_6_coding,m_4_w_7_coding,m_4_w_8_coding
};



// --- from main.cu ---
//  Created by Liu Chengjian on 17/10/9.
//  Copyright (c) 2017 csliu. All rights reserved.
//



// --- from utils.cu ---





// --- from GCRSMatrix.h ---
//
//  GCRSMatrix.h
//  NoSynFree
//
//  Created by Liu Chengjian on 17/8/29.
//  Copyright (c) 2017 csliu. All rights reserved.
//

#ifndef __NoSynFree__GCRSMatrix__
#define __NoSynFree__GCRSMatrix__

#include <stdio.h>
#include <string.h>
#include <math.h>

int gcrs_check_k_m_w(int k, int m, int w);

int *gcrs_create_bitmatrix(int k, int m, int w);

unsigned int *gcrs_create_column_coding_bitmatrix(int k, int m, int w,
                                                       int *bitmatrix);

int gcrs_create_decoding_data_bitmatrix(int k, int m, int w,
                                            int *matrix, int *decoding_data_matrix,
                                            int *erased, int *dm_ids);
int* gcrs_create_decoding_bitmatrix(int k, int m, int w,
                                        int *matrix, int *mat_idx,
                                        int *erasures, int *dm_ids);

int gcrs_generate_coding_vector(int k, int m, int w, int mIdx,
                                    int *matrix, int *invert_matrix, int *vector,
                                    int *erases);

int *gcrs_erasures_to_erased(int k, int m,
                                 int *erasures);

void printMatrix(int *mat, int coloum, int row);
void gcrs_print_column_encoded_bitmatrix(unsigned int *column_encoded_bitmatrix, int k, int m, int w);

#endif /* defined(__NoSynFree__GCRSMatrix__) */


// --- from galois.h ---
/* Galois.h
 * James S. Plank

Galois.tar - Fast Galois Field Arithmetic Library in C/C++
Copright (C) 2007 James S. Plank

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

James S. Plank
Department of Computer Science
University of Tennessee
Knoxville, TN 37996
plank@cs.utk.edu

 */

/*
 * Part of jerasure.h: 
 * $Revision: 1.2 $
 * $Date: 2008/08/19 17:40:58 $
 */

#ifndef _GALOIS_H
#define _GALOIS_H

#include <stdio.h>
#include <stdlib.h>

int galois_single_multiply(int a, int b, int w);
int galois_single_divide(int a, int b, int w);
int galois_log(int value, int w);
int galois_ilog(int value, int w);

int galois_create_log_tables(int w);   /* Returns 0 on success, -1 on failure */
int galois_logtable_multiply(int x, int y, int w);
int galois_logtable_divide(int x, int y, int w);

int galois_create_mult_tables(int w);   /* Returns 0 on success, -1 on failure */
int galois_multtable_multiply(int x, int y, int w);
int galois_multtable_divide(int x, int y, int w);

int galois_shift_multiply(int x, int y, int w);
int galois_shift_divide(int x, int y, int w);

int galois_create_split_w8_tables();
int galois_split_w8_multiply(int x, int y);

int galois_inverse(int x, int w);
int galois_shift_inverse(int y, int w);

int *galois_get_mult_table(int w);
int *galois_get_div_table(int w);
int *galois_get_log_table(int w);
int *galois_get_ilog_table(int w);

void galois_region_xor(           char *r1,         /* Region 1 */
                                  char *r2,         /* Region 2 */
                                  char *r3,         /* Sum region (r3 = r1 ^ r2) -- can be r1 or r2 */
                                  int nbytes);      /* Number of bytes in region */

/* These multiply regions in w=8, w=16 and w=32.  They are much faster
   than calling galois_single_multiply.  The regions must be long word aligned. */

void galois_w08_region_multiply(char *region,       /* Region to multiply */
                                  int multby,       /* Number to multiply by */
                                  int nbytes,       /* Number of bytes in region */
                                  char *r2,         /* If r2 != NULL, products go here.  
                                                       Otherwise region is overwritten */
                                  int add);         /* If (r2 != NULL && add) the produce is XOR'd with r2 */

void galois_w16_region_multiply(char *region,       /* Region to multiply */
                                  int multby,       /* Number to multiply by */
                                  int nbytes,       /* Number of bytes in region */
                                  char *r2,         /* If r2 != NULL, products go here.  
                                                       Otherwise region is overwritten */
                                  int add);         /* If (r2 != NULL && add) the produce is XOR'd with r2 */

void galois_w32_region_multiply(char *region,       /* Region to multiply */
                                  int multby,       /* Number to multiply by */
                                  int nbytes,       /* Number of bytes in region */
                                  char *r2,         /* If r2 != NULL, products go here.  
                                                       Otherwise region is overwritten */
                                  int add);         /* If (r2 != NULL && add) the produce is XOR'd with r2 */

#endif


// --- from jerasure.h ---
/* jerasure.h - header of kernel procedures
 * James S. Plank

JERASURE - Library for Erasure Coding
Copright (C) 2007 James S. Plank

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

James S. Plank
Department of Electrical Engineering and Computer Science
University of Tennessee
Knoxville, TN 37996
plank@cs.utk.edu
*/

/*
 * $Revision: 1.2 $
 * $Date: 2008/08/19 17:40:58 $
 */

#ifndef _JERASURE_H
#define _JERASURE_H

/* This uses procedures from the Galois Field arithmetic library */

//#include "MemTrack.h"

/* ------------------------------------------------------------ */
/* In all of the routines below:

   k = Number of data devices
   m = Number of coding devices
   w = Word size

   data_ptrs = An array of k pointers to data which is size bytes.  
               Size must be a multiple of sizeof(long).
               Pointers must also be longword aligned.
 
   coding_ptrs = An array of m pointers to coding data which is size bytes.

   packetsize = The size of a coding block with bitmatrix coding. 
                When you code with a bitmatrix, you will use w packets
                of size packetsize.

   matrix = an array of k*m integers.  
            It represents an m by k matrix.
            Element i,j is in matrix[i*k+j];

   bitmatrix = an array of k*m*w*w integers.
            It represents an mw by kw matrix.
            Element i,j is in matrix[i*k*w+j];

   erasures = an array of id's of erased devices. 
              Id's are integers between 0 and k+m-1.
              Id's 0 to k-1 are id's of data devices.
              Id's k to k+m-1 are id's of coding devices: 
                  Coding device id = id-k.
              If there are e erasures, erasures[e] = -1.

   schedule = an array of schedule operations.  

              If there are m operations, then schedule[m][0] = -1.

   operation = an array of 5 integers:

          0 = operation: 0 for copy, 1 for xor (-1 for end)
          1 = source device (0 - k+m-1)
          2 = source packet (0 - w-1)
          3 = destination device (0 - k+m-1)
          4 = destination packet (0 - w-1)
 */

/* ---------------------------------------------------------------  */
/* Bitmatrices / schedules ---------------------------------------- */
/*
 - jerasure_matrix_to_bitmatrix turns a m X k matrix in GF(2^w) into a
                              wm X wk bitmatrix (in GF(2)).  This is
                              explained in the Cauchy Reed-Solomon coding
                              paper.

 - jerasure_dumb_bitmatrix_to_schedule turns a bitmatrix into a schedule 
                              using the straightforward algorithm -- just
                              schedule the dot products defined by each
                              row of the matrix.

 - jerasure_smart_bitmatrix_to_schedule turns a bitmatrix into a schedule,
                              but tries to use previous dot products to
                              calculate new ones.  This is the optimization
                              explained in the original Liberation code paper.

 - jerasure_generate_schedule_cache precalcalculate all the schedule for the
                              given distribution bitmatrix.  M must equal 2.
 
 - jerasure_free_schedule frees a schedule that was allocated with 
                              jerasure_XXX_bitmatrix_to_schedule.
 
 - jerasure_free_schedule_cache frees a schedule cache that was created with 
                              jerasure_generate_schedule_cache.
 */

int *jerasure_matrix_to_bitmatrix(int k, int m, int w, int *matrix);
int **jerasure_dumb_bitmatrix_to_schedule(int k, int m, int w, int *bitmatrix);
int **jerasure_smart_bitmatrix_to_schedule(int k, int m, int w, int *bitmatrix);
int ***jerasure_generate_schedule_cache(int k, int m, int w, int *bitmatrix, int smart);

void jerasure_free_schedule(int **schedule);
void jerasure_free_schedule_cache(int k, int m, int ***cache);

/* ------------------------------------------------------------ */
/* Encoding - these are all straightforward.  jerasure_matrix_encode only 
   works with w = 8|16|32.  */

void jerasure_do_parity(int k, char **data_ptrs, char *parity_ptr, int size);

void jerasure_matrix_encode(int k, int m, int w, int *matrix,
                          char **data_ptrs, char **coding_ptrs, int size);

void jerasure_bitmatrix_encode(int k, int m, int w, int *bitmatrix,
                            char **data_ptrs, char **coding_ptrs, int size, int packetsize);
void jerasure_schedule_encode(int k, int m, int w, int **schedule,
                                  char **data_ptrs, char **coding_ptrs, int size, int packetsize);

/* ------------------------------------------------------------ */
/* Decoding. -------------------------------------------------- */

/* These return integers, because the matrix may not be invertible. 
   
   The parameter row_k_ones should be set to 1 if row k of the matrix
   (or rows kw to (k+1)w+1) of th distribution matrix are all ones
   (or all identity matrices).  Then you can improve the performance
   of decoding when there is more than one failure, and the parity
   device didn't fail.  You do it by decoding all but one of the data
   devices, and then decoding the last data device from the data devices
   and the parity device.

   jerasure_schedule_decode_lazy generates the schedule on the fly.

   jerasure_matrix_decode only works when w = 8|16|32.

   jerasure_make_decoding_matrix/bitmatrix make the k*k decoding matrix
         (or wk*wk bitmatrix) by taking the rows corresponding to k
         non-erased devices of the distribution matrix, and then
         inverting that matrix.

         You should already have allocated the decoding matrix and
         dm_ids, which is a vector of k integers.  These will be
         filled in appropriately.  dm_ids[i] is the id of element
         i of the survivors vector.  I.e. row i of the decoding matrix
         times dm_ids equals data drive i.

         Both of these routines take "erased" instead of "erasures".
         Erased is a vector with k+m elements, which has 0 or 1 for 
         each device's id, according to whether the device is erased.
 
   jerasure_erasures_to_erased allocates and returns erased from erasures.
    
 */

int jerasure_matrix_decode(int k, int m, int w, 
                          int *matrix, int row_k_ones, int *erasures,
                          char **data_ptrs, char **coding_ptrs, int size);
                          
int jerasure_bitmatrix_decode(int k, int m, int w, 
                            int *bitmatrix, int row_k_ones, int *erasures,
                            char **data_ptrs, char **coding_ptrs, int size, int packetsize);

int jerasure_schedule_decode_lazy(int k, int m, int w, int *bitmatrix, int *erasures,
                            char **data_ptrs, char **coding_ptrs, int size, int packetsize,
                            int smart);

int jerasure_schedule_decode_cache(int k, int m, int w, int ***scache, int *erasures,
                            char **data_ptrs, char **coding_ptrs, int size, int packetsize);

int jerasure_make_decoding_matrix(int k, int m, int w, int *matrix, int *erased, 
                                  int *decoding_matrix, int *dm_ids);

int jerasure_make_decoding_bitmatrix(int k, int m, int w, int *matrix, int *erased, 
                                  int *decoding_matrix, int *dm_ids);

int *jerasure_erasures_to_erased(int k, int m, int *erasures);

/* ------------------------------------------------------------ */
/* These perform dot products and schedules. -------------------*/
/*
   src_ids is a matrix of k id's (0 - k-1 for data devices, k - k+m-1
   for coding devices) that identify the source devices.  Dest_id is
   the id of the destination device.

   jerasure_matrix_dotprod only works when w = 8|16|32.

   jerasure_do_scheduled_operations executes the schedule on w*packetsize worth of
   bytes from each device.  ptrs is an array of pointers which should have as many
   elements as the highest referenced device in the schedule.

 */
 
void jerasure_matrix_dotprod(int k, int w, int *matrix_row,
                          int *src_ids, int dest_id,
                          char **data_ptrs, char **coding_ptrs, int size);

void jerasure_bitmatrix_dotprod(int k, int w, int *bitmatrix_row,
                             int *src_ids, int dest_id,
                             char **data_ptrs, char **coding_ptrs, int size, int packetsize);

void jerasure_do_scheduled_operations(char **ptrs, int **schedule, int packetsize);

/* ------------------------------------------------------------ */
/* Matrix Inversion ------------------------------------------- */
/*
   The two matrix inversion functions work on rows*rows matrices of
   ints.  If a bitmatrix, then each int will just be zero or one.
   Otherwise, they will be elements of gf(2^w).  Obviously, you can
   do bit matrices with crs_invert_matrix() and set w = 1, but
   crs_invert_bitmatrix will be more efficient.

   The two invertible functions return whether a matrix is invertible.
   They are more efficient than the inverstion functions.

   Mat will be destroyed when the matrix inversion or invertible
   testing is done.  Sorry.

   Inv must be allocated by the caller.

   The two invert_matrix functions return 0 on success, and -1 if the
   matrix is uninvertible.

   The two invertible function simply return whether the matrix is
   invertible.  (0 or 1). Mat will be destroyed.
 */

int jerasure_invert_matrix(int *mat, int *inv, int rows, int w);
int jerasure_invert_bitmatrix(int *mat, int *inv, int rows);
int jerasure_invertible_matrix(int *mat, int rows, int w);
int jerasure_invertible_bitmatrix(int *mat, int rows);

/* ------------------------------------------------------------ */
/* Basic matrix operations -------------------------------------*/
/*
   Each of the print_matrix routines require a w.  In jerasure_print_matrix,
   this is to calculate the field width.  In jerasure_print_bitmatrix, it is
   to put spaces between the bits.

   jerasure_matrix_multiply is a simple matrix multiplier in GF(2^w).  It returns a r1*c2
   matrix, which is the product of the two input matrices.  It allocates
   the product.  Obviously, c1 should equal r2.  However, this is not
   validated by the procedure.  
*/

void jerasure_print_matrix(int *matrix, int rows, int cols, int w);
void jerasure_print_bitmatrix(int *matrix, int rows, int cols, int w);

int *jerasure_matrix_multiply(int *m1, int *m2, int r1, int c1, int r2, int c2, int w);

/* ------------------------------------------------------------ */
/* Stats ------------------------------------------------------ */
/*
  jerasure_get_stats fills in a vector of three doubles:

      fill_in[0] is the number of bytes that have been XOR'd
      fill_in[1] is the number of bytes that have been copied
      fill_in[2] is the number of bytes that have been multiplied
                 by a constant in GF(2^w)

  When jerasure_get_stats() is called, it resets its values.
 */

void jerasure_get_stats(double *fill_in);

#endif


// --- from utils.h ---
#ifndef __UTILS
#define __UTILS

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

size_t align_value(size_t valueToAlign, size_t alignMask);

void generateRandomValue(char *data, size_t size);
typedef void (*coding_func)(int k, int index,
    char *dataPtr, char *codeDevPtr,
    const unsigned int *bitMatrixPtr,
    int threadDimX,int blockDimX,
    int workSizePerGridInLong);

double elapsed_time_in_ms(struct timeval startTime, struct timeval endTime);
 

#define talloc(type, num) (type *)malloc(sizeof(type) * (num))

#define MIN_K 1
#define MAX_K 4

#define MIN_M 1
#define MAX_M 4

#define MIN_W 4
#define MAX_W 8

#define MAX_K_MULIPLY_M (MAX_K * MAX_M)

#define WARM_UP_SIZE 4

#define MAX_THREAD_NUM 128
#define THREADS_PER_BLOCK 128
#define WARP_SIZE 32

#endif

