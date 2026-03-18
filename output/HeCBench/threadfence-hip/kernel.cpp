#include "kernel.h"

// --- from main.cu ---
extern "C"
void sum (
    const float* array,
    const int N,
    unsigned int * count,
    volatile float* result)
{
    #pragma HLS INTERFACE m_axi port=array offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE m_axi port=count offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            bool isLastBlockDone;
            float partialSum;

            // Each block sums a subset of the input array.
            unsigned int bid = _bid_x;
            unsigned int num_blocks = GRID_DIM_X;
            unsigned int block_size = BLOCK_DIM_X;
            unsigned int lid = _tid_x;
            unsigned int gid = bid * block_size + lid;

            if (lid == 0) partialSum = 0;

            if (gid < N)
            (partialSum += array[gid]);

            if (lid == 0) {

            // Thread 0 of each block stores the partial sum
            // to global memory. The compiler will use
            // a store operation that bypasses the L1 cache
            // since the "result" variable is declared as
            // volatile. This ensures that the threads of
            // the last block will read the correct partial
            // sums computed by all other blocks.
            result[bid] = partialSum;

            // Thread 0 makes sure that the incrementation
            // of the "count" variable is only performed after
            // the partial sum has been written to global memory.
            __threadfence();

            // Thread 0 signals that it is done.
            unsigned int value = (*count += 1);

            // Thread 0 determines if its block is the last
            // block to be done.
            isLastBlockDone = (value == (num_blocks - 1));
            }

            // Synchronize to make sure that each thread reads
            // the correct value of isLastBlockDone.

            if (isLastBlockDone) {

            // The last block sums the partial sums
            // stored in result[0 .. num_blocks-1]
            if (lid == 0) partialSum = 0;

            for (int i = lid; i < num_blocks; i += block_size)
            (partialSum += result[i]);

            if (lid == 0) {
            // Thread 0 of last block stores the total sum
            // to global memory and resets the count
            // varialble, so that the next kernel call
            // works properly.
            result[0] = partialSum;
            *count = 0;
            }
            }

        }
    }
}
