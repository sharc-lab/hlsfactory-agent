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

// --- from masking.cu ---
/****
  DIAMOND protein aligner
  Copyright (C) 2013-2017 Benjamin Buchfink <buchfink@gmail.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****/
#include "../diamond-sycl/src/basic/masking.h"

#define SEQ_LEN 33





auto_ptr<Masking> Masking::instance;
const uint8_t Masking::bit_mask = 128;

Masking::Masking(const Score_matrix &score_matrix)
{
  const double lambda = score_matrix.lambda(); // 0.324032
  std::copy(likelihoodRatioMatrix_, likelihoodRatioMatrix_ + size, probMatrixPointers_);
  int firstGapCost = score_matrix.gap_extend() + score_matrix.gap_open();
  firstGapProb_ = exp(-lambda * firstGapCost);
  otherGapProb_ = exp(-lambda * score_matrix.gap_extend());
  firstGapProb_ /= (1 - otherGapProb_);
}

void Masking::operator()(Letter *seq, size_t len) const
{

  tantan::maskSequences((tantan::uchar*)seq, (tantan::uchar*)(seq + len), 50,
      (tantan::const_double_ptr*)probMatrixPointers_,
      0.005, 0.05,
      0.9,
      0, 0,
      0.5, (const tantan::uchar*)mask_table_x_);
}

unsigned char* Masking::call_opt(Sequence_set &seqs) const
{
  const int n = seqs.get_length();
  int total = 0;
  for (int i=0; i < n; i++)
    total += seqs.length(i);

  printf("There are %d sequences and the total sequence length is %d\n", n, total);
  unsigned char *seqs_device = NULL;
  posix_memalign((void**)&seqs_device, 1024, total);

  unsigned char *p = seqs_device;

  double *probMat_device = NULL;
  posix_memalign((void**)&probMat_device, 1024, size*size*sizeof(double));
  for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++)
      probMat_device[i*size+j] = probMatrixPointers_[i][j];

  unsigned char *mask_table_device = NULL;
  posix_memalign((void**)&mask_table_device, 1024, size*sizeof(unsigned char));
  for (int i = 0; i < size; i++)
    mask_table_device[i] = mask_table_x_[i];

  int len = 33;

  printf("Timing the mask sequences on device...\n");
  Timer t;
  t.start();

  const int size = len;
  const int maxRepeatOffset = 50;
  const double repeatProb = 0.005;
  const double repeatEndProb = 0.05;
  const double repeatOffsetProbDecay = 0.9;
  const double firstGapProb = 0;
  const double otherGapProb = 0;
  const double minMaskProb = 0.5;
  const int seqs_len = n;

  unsigned char* d_seqs;

  unsigned char* d_maskTable;

  double* d_probMat;

  dim3 grids ((seqs_len+128)/128);
  dim3 threads (128);

      d_seqs,
      d_probMat,
      d_maskTable,
      size,
      maxRepeatOffset,
      repeatProb,
      repeatEndProb,
      repeatOffsetProbDecay,
      firstGapProb,
      otherGapProb,
      minMaskProb,
      seqs_len);

  message_stream << "Total time (maskSequences) on the device = " <<
    t.getElapsedTimeInMicroSec() / 1e6 << " s" << std::endl;

  free(probMat_device);
  free(mask_table_device);
  return seqs_device;
}

void Masking::call_opt(Letter *seq, size_t len) const
{
  // CPU
  tantale::maskSequences((tantan::uchar*)seq, (tantan::uchar*)(seq + len), 50,
      (tantan::const_double_ptr*)probMatrixPointers_,
      0.005, 0.05,
      0.9,
      0, 0,
      0.5, (const tantan::uchar*)mask_table_x_);
}

void Masking::mask_bit(Letter *seq, size_t len) const
{

  tantan::maskSequences((tantan::uchar*)seq, (tantan::uchar*)(seq + len), 50,
      (tantan::const_double_ptr*)probMatrixPointers_,
      0.005, 0.05,
      0.9,
      0, 0,
      0.5,		(const tantan::uchar*)mask_table_bit_);
}

void Masking::bit_to_hard_mask(Letter *seq, size_t len, size_t &n) const
{
}

void Masking::remove_bit_mask(Letter *seq, size_t len) const
{
  for (size_t i = 0; i < len; ++i)
    if (seq[i] & bit_mask)
      seq[i] &= ~bit_mask;
}


