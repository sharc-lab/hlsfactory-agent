#include "kernel.h"

// --- from main.cu ---
extern "C"
void ScanLargeArrays(float * output,
                     const float * input,
                     const unsigned int block_size,   // size of block
                     float * sumBuffer)  // sum of blocks
{
  float block[4096];   // Size : block_size
  int tid = _tid_x;
  int bid = _bid_x;
  int gid = bid * BLOCK_DIM_X + tid;

  /* Cache the computational window in shared memory */
  block[2*tid]     = input[2*gid];
  block[2*tid + 1] = input[2*gid + 1];

  float cache0 = block[0];
  float cache1 = cache0 + block[1];

  /* build the sum in place up the tree */
  for(int stride = 1; stride < block_size; stride *=2)
  {
    if(2*tid>=stride)
    {
      cache0 = block[2*tid-stride]+block[2*tid];
      cache1 = block[2*tid+1-stride]+block[2*tid+1];
    }

    block[2*tid] = cache0;
    block[2*tid+1] = cache1;
  }

  /* store the value in sum buffer before making it to 0 */   
  sumBuffer[bid] = block[block_size-1];

  /*write the results back to global memory */
  if(tid==0)
  {
    output[2*gid]     = 0;
    output[2*gid+1]   = block[2*tid];
  }
  else
  {
    output[2*gid]     = block[2*tid-1];
    output[2*gid + 1] = block[2*tid];
  }
}
extern "C"

void prefixSum(float * output, 
               const float * input,
               const unsigned int block_size)
{
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=block_size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int tid = _tid_x;
            int bid = _bid_x;
            int gid = bid * BLOCK_DIM_X + tid;

            float block[4096];

            /* Cache the computational window in shared memory */
            block[2*tid]     = input[2*gid];
            block[2*tid + 1] = input[2*gid + 1];

            float cache0 = block[0];
            float cache1 = cache0 + block[1];

            /* build the sum in place up the tree */
            for(int stride = 1; stride < block_size; stride *=2)
            {

            if(2*tid>=stride)
            {
            cache0 = block[2*tid-stride]+block[2*tid];
            cache1 = block[2*tid+1-stride]+block[2*tid+1];
            }

            block[2*tid] = cache0;
            block[2*tid+1] = cache1;
            }

            /*write the results back to global memory */
            if(tid==0)
            {
            output[2*gid]     = 0;
            output[2*gid+1]   = block[2*tid];
            }
            else
            {
            output[2*gid]     = block[2*tid-1];
            output[2*gid + 1] = block[2*tid];
            }

        }
    }
}
