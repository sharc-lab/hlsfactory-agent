#include "kernel.h"

// --- from main.cu ---
uint4 scan4(const uint4 idata, unsigned int* ptr)
{    

  unsigned int idx = _tid_x;

  uint4 val4 = idata;
  unsigned int sum[3];
  sum[0] = val4.x;
  sum[1] = val4.y + sum[0];
  sum[2] = val4.z + sum[1];

  unsigned int val = val4.w + sum[2];

  val = scanwarp(val, ptr, 4);

  if ((idx & (WARP_SIZE - 1)) == WARP_SIZE - 1)
  {
    ptr[idx >> 5] = val + val4.w + sum[2];
  }

  if (idx < WARP_SIZE)
    ptr[idx] = scanwarp(ptr[idx], ptr, 2);

  val += ptr[idx >> 5];

  val4.x = val;
  val4.y = val + sum[0];
  val4.z = val + sum[1];
  val4.w = val + sum[2];

  return val4;
}

uint4 rank4(const uint4 preds, unsigned int* sMem, unsigned int* numtrue)
{
  int localId = _tid_x;
  int localSize = BLOCK_DIM_X;

  uint4 address = scan4(preds, sMem);

  if (localId == localSize - 1) 
  {
    numtrue[0] = address.w + preds.w;
  }

  uint4 rank;
  int idx = localId*4;
  rank.x = (preds.x) ? address.x : numtrue[0] + idx - address.x;
  rank.y = (preds.y) ? address.y : numtrue[0] + idx + 1 - address.y;
  rank.z = (preds.z) ? address.z : numtrue[0] + idx + 2 - address.z;
  rank.w = (preds.w) ? address.w : numtrue[0] + idx + 3 - address.w;

  return rank;
}
extern "C"

void radixSortBlocksKeysK(
   unsigned int* keysIn, 
   unsigned int* keysOut,
   const unsigned int nbits,
   const unsigned int startbit)
{
    #pragma HLS INTERFACE m_axi port=keysIn offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=keysOut offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=nbits
    #pragma HLS INTERFACE s_axilite port=startbit
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=numtrue complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sMem complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int globalId = _bid_x * BLOCK_DIM_X + _tid_x;
            unsigned int numtrue[1];
            unsigned int sMem[4*128];

            uint4 key = reinterpret_cast<uint4*>(keysIn)[globalId];

            // radixSortBlockKeysOnly(&key, nbits, startbit, sMem, numtrue);
            int localId = _tid_x;
            int localSize = BLOCK_DIM_X;

            for(unsigned int shift = startbit; shift < (startbit + nbits); ++shift)
            {
            uint4 lsb;
            lsb.x = !((key.x >> shift) & 0x1);
            lsb.y = !((key.y >> shift) & 0x1);
            lsb.z = !((key.z >> shift) & 0x1);
            lsb.w = !((key.w >> shift) & 0x1);

            uint4 r;

            r = rank4(lsb, sMem, numtrue);

            // This arithmetic strides the ranks across 4 CTA_SIZE regions
            sMem[(r.x & 3) * localSize + (r.x >> 2)] = key.x;
            sMem[(r.y & 3) * localSize + (r.y >> 2)] = key.y;
            sMem[(r.z & 3) * localSize + (r.z >> 2)] = key.z;
            sMem[(r.w & 3) * localSize + (r.w >> 2)] = key.w;

            // The above allows us to read without 4-way bank conflicts:
            key.x = sMem[localId];
            key.y = sMem[localId +     localSize];
            key.z = sMem[localId + 2 * localSize];
            key.w = sMem[localId + 3 * localSize];
            }

            //keysOut[globalId] = key;
            reinterpret_cast<uint4*>(keysOut)[globalId] = key;


        }
    }
}
