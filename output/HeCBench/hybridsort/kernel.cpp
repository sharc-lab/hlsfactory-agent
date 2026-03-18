#include "kernel.h"

// --- from mergesort.cu ---
float4 sortElem(float4 r) {
  float4 nr;

  float xt = r.x;
  float yt = r.y;
  float zt = r.z;
  float wt = r.w;

  float nr_xt = xt > yt ? yt : xt;
  float nr_yt = yt > xt ? yt : xt;
  float nr_zt = zt > wt ? wt : zt;
  float nr_wt = wt > zt ? wt : zt;

  xt = nr_xt > nr_zt ? nr_zt : nr_xt;
  yt = nr_yt > nr_wt ? nr_wt : nr_yt;
  zt = nr_zt > nr_xt ? nr_zt : nr_xt;
  wt = nr_wt > nr_yt ? nr_wt : nr_yt;

  nr.x = xt;
  nr.y = yt > zt ? zt : yt;
  nr.z = zt > yt ? zt : yt;
  nr.w = wt;
  return nr;
}
extern "C"

  void sortElement(float4* result, float4* input, const int size) 
{
    #pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                int gid = _bid_x*BLOCK_DIM_X+_tid_x;
                if (gid < size) result[gid] = sortElem(input[gid]);

            }
        }
    }
}

  float4 getLowest(float4 a, float4 b)
{
  float ax = a.x;
  float ay = a.y;
  float az = a.z;
  float aw = a.w;
  float bx = b.x;
  float by = b.y;
  float bz = b.z;
  float bw = b.w;
  a.x = ax < bw ? ax : bw;
  a.y = ay < bz ? ay : bz;
  a.z = az < by ? az : by;
  a.w = aw < bx ? aw : bx;
  return a;
}

  float4 getHighest(float4 a, float4 b)
{
  float ax = a.x;
  float ay = a.y;
  float az = a.z;
  float aw = a.w;
  float bx = b.x;
  float by = b.y;
  float bz = b.z;
  float bw = b.w;
  b.x = aw >= bx ? aw : bx;
  b.y = az >= by ? az : by;
  b.z = ay >= bz ? ay : bz;
  b.w = ax >= bw ? ax : bw;
  return b;
}
extern "C"

  void mergepack ( float* result , 
    const float* orig , 
    const int *constStartAddr,
    const unsigned int *finalStartAddr,
    const unsigned int *nullElems )
{
    #pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=orig offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=constStartAddr offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=finalStartAddr offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=nullElems offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1


                const int gid = _bid_x * BLOCK_DIM_X + _tid_x;
                int division = _bid_y;
                if((finalStartAddr[division] + gid) < finalStartAddr[division + 1])
                result[finalStartAddr[division] + gid] =
                orig[constStartAddr[division]*4 + nullElems[division] + gid];

            }
        }
    }
}
