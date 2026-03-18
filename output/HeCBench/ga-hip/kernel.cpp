#include "kernel.h"

// --- from main.cu ---
extern "C"
void ga(const char * target,
        const char * query,
              char * batch_result,
              uint32_t length,
              int query_sequence_length,
              int coarse_match_length,
              int coarse_match_threshold,
              int current_position)
{
    #pragma HLS INTERFACE m_axi port=target offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=query offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=batch_result offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=length
    #pragma HLS INTERFACE s_axilite port=query_sequence_length
    #pragma HLS INTERFACE s_axilite port=coarse_match_length
    #pragma HLS INTERFACE s_axilite port=coarse_match_threshold
    #pragma HLS INTERFACE s_axilite port=current_position
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            uint tid = _bid_x * BLOCK_DIM_X + _tid_x;
            if (tid > length) return;
            bool match = false;
            int max_length = query_sequence_length - coarse_match_length;

            for (int i = 0; i <= max_length; i++) {
            int distance = 0;
            for (int j = 0; j < coarse_match_length; j++) {
            if (target[current_position + tid + j] != query[i + j]) {
            distance++;
            }
            }

            if (distance < coarse_match_threshold) {
            match = true;
            break;
            }
            }
            if (match) {
            batch_result[tid] = 1;
            }

        }
    }
}
