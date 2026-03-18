#include "kernel.h"

// --- from main.cu ---
extern "C"
void reduceFinal(const double *inputVec,
                            double *result,
                            size_t inputSize)
{
    #pragma HLS INTERFACE m_axi port=inputVec offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=inputSize
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=tmp complete dim=1
    #pragma HLS ARRAY_PARTITION variable=tmp complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            double tmp[THREADS_PER_BLOCK];

            size_t globaltid = (size_t)_bid_x*BLOCK_DIM_X + _tid_x;

            double temp_sum = 0.0;
            for (size_t i = globaltid; i < inputSize; i += GRID_DIM_X*BLOCK_DIM_X)
            {
            temp_sum += inputVec[i];
            }
            tmp[cta.thread_rank()] = temp_sum;

            // do reduction in shared mem
            if ((BLOCK_DIM_X >= 512) && (cta.thread_rank() < 256))
            {
            tmp[cta.thread_rank()] = temp_sum = temp_sum + tmp[cta.thread_rank() + 256];
            }

            if ((BLOCK_DIM_X >= 256) &&(cta.thread_rank() < 128))
            {
            tmp[cta.thread_rank()] = temp_sum = temp_sum + tmp[cta.thread_rank() + 128];
            }

            if ((BLOCK_DIM_X >= 128) && (cta.thread_rank() <  64))
            {
            tmp[cta.thread_rank()] = temp_sum = temp_sum + tmp[cta.thread_rank() +  64];
            }

            if (cta.thread_rank() < 32)
            {
            // Fetch final intermediate sum from 2nd warp
            if (BLOCK_DIM_X >=  64) temp_sum += tmp[cta.thread_rank() + 32];
            // Reduce final warp using shuffle
            for (int offset = tile32.size()/2; offset > 0; offset /= 2)
            {
            temp_sum += tile32.shfl_down(temp_sum, offset);
            }
            }
            // write result for this block to global mem
            if (cta.thread_rank() == 0) result[0] = temp_sum;

        }
    }
}
