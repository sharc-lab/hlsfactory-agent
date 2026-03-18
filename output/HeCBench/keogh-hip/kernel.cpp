#include "kernel.h"

// --- from main.cu ---
extern "C"
void lb_keogh(const float * subject,
              const float * avgs,
              const float * stds, 
                    float * lb_keogh,
              const float * lower_bound,
              const float * upper_bound,
              const int M,
              const int N) 
{
    #pragma HLS INTERFACE m_axi port=subject offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=avgs offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=stds offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=lb_keogh offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=lower_bound offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=upper_bound offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=cache complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            // shared memory
            float cache[4096];

            int lid = _tid_x;
            int blockSize = BLOCK_DIM_X * _bid_x;
            int idx = blockSize + lid;

            for (int k = lid; k < BLOCK_DIM_X + M; k += BLOCK_DIM_X)
            if (blockSize + k < N) cache[k] = subject[blockSize + k];

            if (idx < N-M+1) {

            // obtain statistics
            float residues = 0;
            float avg = avgs[idx];
            float std = stds[idx];

            for (int i = 0; i < M; ++i) {
            // differences to envelopes
            float value = (cache[lid+i] - avg) / std;
            float lower = value - lower_bound[i];
            float upper = value - upper_bound[i];

            // Euclidean or Manhattan distance?
            residues += upper*upper*(upper > 0) + lower*lower*(lower < 0);
            }

            lb_keogh[idx] = residues;
            }

        }
    }
}
