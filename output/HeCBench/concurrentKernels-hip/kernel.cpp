#include "kernel.h"

// --- from main.cu ---
extern "C"
void clock_block(long *d_o, long clock_count) {
    #pragma HLS INTERFACE m_axi port=d_o offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=clock_count
    #pragma HLS INTERFACE s_axilite port=return

    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
    #pragma HLS PIPELINE II=1

        long clock_offset = 0;
        for (int i = 0; i < clock_count; i++)
        clock_offset += i % 3;
        d_o[0] = clock_offset;

    }
}
extern "C"

void sum(long *d_clocks, int N) {
    #pragma HLS INTERFACE m_axi port=d_clocks offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_clocks complete dim=1

    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
    #pragma HLS PIPELINE II=1

        // Handle to thread block group
        long s_clocks[32];

        long my_sum = 0;

        for (int i = _tid_x; i < N; i += BLOCK_DIM_X) {
        my_sum += d_clocks[i];
        }

        s_clocks[_tid_x] = my_sum;

        for (int i = 16; i > 0; i /= 2) {
        if (_tid_x < i) {
        s_clocks[_tid_x] += s_clocks[_tid_x + i];
        }
        }

        d_clocks[0] = s_clocks[0];

    }
}
