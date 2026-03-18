#include "kernel.h"

// --- from RadixSort_kernels.cu ---
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
   const unsigned int* keysIn,
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
            unsigned int sMem[4*CTA_SIZE];

            uint4 key = reinterpret_cast<const uint4*>(keysIn)[globalId];

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
extern "C"

void findRadixOffsetsK(
    const unsigned int* keys,
          unsigned int* counters,
          unsigned int* blockOffsets,
    const unsigned int startbit,
    const unsigned int totalBlocks)
{
    #pragma HLS INTERFACE m_axi port=keys offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=counters offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=blockOffsets offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=startbit
    #pragma HLS INTERFACE s_axilite port=totalBlocks
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sStartPointers complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sRadix1 complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned int  sStartPointers[16];
            unsigned int  sRadix1[2*CTA_SIZE];

            unsigned int groupId = _bid_x;
            unsigned int localId = _tid_x;
            unsigned int groupSize = BLOCK_DIM_X;
            unsigned int globalId = groupId * groupSize + localId;

            uint2 radix2 = reinterpret_cast<const uint2*>(keys)[globalId];

            sRadix1[2 * localId]     = (radix2.x >> startbit) & 0xF;
            sRadix1[2 * localId + 1] = (radix2.y >> startbit) & 0xF;

            // Finds the position where the sRadix1 entries differ and stores start
            // index for each radix.
            if(localId < 16)
            {
            sStartPointers[localId] = 0;
            }

            if((localId > 0) && (sRadix1[localId] != sRadix1[localId - 1]) )
            {
            sStartPointers[sRadix1[localId]] = localId;
            }
            if(sRadix1[localId + groupSize] != sRadix1[localId + groupSize - 1])
            {
            sStartPointers[sRadix1[localId + groupSize]] = localId + groupSize;
            }

            if(localId < 16)
            {
            blockOffsets[groupId*16 + localId] = sStartPointers[localId];
            }

            // Compute the sizes of each block.
            if((localId > 0) && (sRadix1[localId] != sRadix1[localId - 1]) )
            {
            sStartPointers[sRadix1[localId - 1]] =
            localId - sStartPointers[sRadix1[localId - 1]];
            }
            if(sRadix1[localId + groupSize] != sRadix1[localId + groupSize - 1] )
            {
            sStartPointers[sRadix1[localId + groupSize - 1]] =
            localId + groupSize - sStartPointers[sRadix1[localId + groupSize - 1]];
            }

            if(localId == groupSize - 1)
            {
            sStartPointers[sRadix1[2 * groupSize - 1]] =
            2 * groupSize - sStartPointers[sRadix1[2 * groupSize - 1]];
            }

            if(localId < 16)
            {
            counters[localId * totalBlocks + groupId] = sStartPointers[localId];
            }

        }
    }
}
extern "C"

void reorderDataKeysOnlyK(
          unsigned int* outKeys,
    const unsigned int* keys,
          unsigned int* blockOffsets,
    const unsigned int* offsets,
    const unsigned int startbit,
    const unsigned int numElements,
    const unsigned int totalBlocks)
{
    #pragma HLS INTERFACE m_axi port=outKeys offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=keys offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=blockOffsets offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=offsets offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=startbit
    #pragma HLS INTERFACE s_axilite port=numElements
    #pragma HLS INTERFACE s_axilite port=totalBlocks
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sOffsets complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sBlockOffsets complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sKeys2 complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned int sOffsets[16];
            unsigned int sBlockOffsets[16];
            uint2 sKeys2[CTA_SIZE];

            unsigned int *sKeys1 = (unsigned int*)sKeys2;

            unsigned int groupId = _bid_x;
            unsigned int localId = _tid_x;
            unsigned int groupSize = BLOCK_DIM_X;
            unsigned int globalId = groupId * groupSize + localId;

            sKeys2[localId] = reinterpret_cast<const uint2*>(keys)[globalId];

            if(localId < 16)
            {
            sOffsets[localId]      = offsets[localId * totalBlocks + groupId];
            sBlockOffsets[localId] = blockOffsets[groupId * 16 + localId];
            }

            unsigned int radix = (sKeys1[localId] >> startbit) & 0xF;
            unsigned int globalOffset = sOffsets[radix] + localId - sBlockOffsets[radix];

            if (globalOffset < numElements)
            {
            outKeys[globalOffset]   = sKeys1[localId];
            }

            radix = (sKeys1[localId + groupSize] >> startbit) & 0xF;
            globalOffset = sOffsets[radix] + localId + groupSize - sBlockOffsets[radix];

            if (globalOffset < numElements)
            {
            outKeys[globalOffset]   = sKeys1[localId + groupSize];
            }

        }
    }
}


// --- from Scan_kernels.cu ---
inline unsigned int scan1Inclusive(const unsigned int idata, 
                           unsigned int* l_Data, const unsigned int size)
{
  unsigned int pos = 2 * _tid_x - (_tid_x & (size - 1));
  l_Data[pos] = 0;
  pos += size;
  l_Data[pos] = idata;

  for(unsigned int offset = 1; offset < size; offset <<= 1){
    unsigned int t = l_Data[pos] + l_Data[pos - offset];
    l_Data[pos] = t;
  }

  return l_Data[pos];
}

inline unsigned int scan1Exclusive(const unsigned int idata, 
                                   unsigned int* l_Data, const unsigned int size)
{
  return scan1Inclusive(idata, l_Data, size) - idata;
}

inline unsigned int scan1Inclusive(const unsigned int idata, 
                                   unsigned int* l_Data, const unsigned int size)
{
  if(size > WARP_SIZE){
    //Bottom-level inclusive warp scan
    unsigned int warpResult = warpScanInclusive(idata, l_Data, WARP_SIZE);

    //Save top elements of each warp for exclusive warp scan
    //sync to wait for warp scans to complete (because l_Data is being overwritten)

    int lid = _tid_x;
    if( (lid & (WARP_SIZE - 1)) == (WARP_SIZE - 1) )
      l_Data[lid >> LOG2_WARP_SIZE] = warpResult;

    //wait for warp scans to complete
    if( lid < (WORKGROUP_SIZE / WARP_SIZE) ){
      //grab top warp elements
      unsigned int val = l_Data[lid] ;
      //calculate exclsive scan and write back to shared memory
      l_Data[lid] = warpScanExclusive(val, l_Data, size >> LOG2_WARP_SIZE);
    }

    //return updated warp scans with exclusive scan results
    return warpResult + l_Data[lid >> LOG2_WARP_SIZE];
  }else{
    return warpScanInclusive(idata, l_Data, size);
  }
}

inline unsigned int scan1Exclusive(const unsigned int idata, 
                                   unsigned int* l_Data, const unsigned int size){
  return scan1Inclusive(idata, l_Data, size) - idata;
}

inline uint4 scan4Inclusive(uint4 data4, 
                            unsigned int* l_Data, const unsigned int size){
  //Level-0 inclusive scan
  data4.y += data4.x;
  data4.z += data4.y;
  data4.w += data4.z;

  //Level-1 exclusive scan
  unsigned int val = scan1Inclusive(data4.w, l_Data, size / 4) - data4.w;

  return (data4 + make_uint4(val));
}

inline uint4 scan4Exclusive(uint4 data4, 
                            unsigned int* l_Data, const unsigned int size)
{
  return scan4Inclusive(data4, l_Data, size) - data4;
}
extern "C"

void scanExclusiveLocal1K(
            unsigned int* d_Dst,
      const unsigned int* d_Src,
      const unsigned int size)
{
    #pragma HLS INTERFACE m_axi port=d_Dst offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_Src offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=l_Data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=l_Data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned int l_Data[2 * WORKGROUP_SIZE];
            int i = _bid_x * BLOCK_DIM_X + _tid_x;

            //Load data
            uint4 idata4 = reinterpret_cast<const uint4*>(d_Src)[i];

            //Calculate exclusive scan
            uint4 odata4 = scan4Exclusive(idata4, l_Data, size);

            //Write back
            reinterpret_cast<uint4*>(d_Dst)[i] = odata4;

        }
    }
}
extern "C"

void scanExclusiveLocal2K(
            unsigned int* d_Buf,
            unsigned int* d_Dst,
      const unsigned int* d_Src,
      const unsigned int N,
      const unsigned int arrayLength)
{
    #pragma HLS INTERFACE m_axi port=d_Buf offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_Dst offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_Src offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=arrayLength
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=l_Data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=l_Data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            //Load top elements
            //Convert results of bottom-level scan back to inclusive
            //Skip loads and stores for inactive work-items of the work-group with highest index(pos >= N)

            unsigned int l_Data[2 * WORKGROUP_SIZE];
            unsigned int data = 0;
            int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if(i < N)
            data = d_Dst[(4 * WORKGROUP_SIZE - 1) + (4 * WORKGROUP_SIZE) * i] +
            d_Src[(4 * WORKGROUP_SIZE - 1) + (4 * WORKGROUP_SIZE) * i];

            //Compute
            unsigned int odata = scan1Exclusive(data, l_Data, arrayLength);

            //Avoid out-of-bound access
            if(i < N) d_Buf[i] = odata;

        }
    }
}
extern "C"

void uniformUpdateK(
      unsigned int* d_Data,
      unsigned int* d_Buf)
{
    #pragma HLS INTERFACE m_axi port=d_Data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_Buf offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=buf complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned int buf[1];

            int i = _bid_x * BLOCK_DIM_X + _tid_x;
            uint4 data4 = reinterpret_cast<uint4*>(d_Data)[i];
            if(_tid_x == 0)
            buf[0] = d_Buf[_bid_x];
            data4 += make_uint4(buf[0]);

            reinterpret_cast<uint4*>(d_Data)[i] = data4;

        }
    }
}
