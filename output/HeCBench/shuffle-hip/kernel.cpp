#include "kernel.h"

// --- from main.cu ---
extern "C"
void bcast_shfl_sg8(const int arg, int *out) {
    #pragma HLS INTERFACE s_axilite port=arg
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int value = ((_tid_x & 0x7) == 0) ? arg : 0;
            // Synchronize all threads in warp, and get "value" from lane 0
            int out_v = __shfl( value, 0);
            size_t oi = BLOCK_DIM_X * _bid_x + _tid_x;
            out[oi] = out_v;

        }
    }
}
extern "C"

void bcast_shfl_xor_sg8(int *out) {
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int value = (_tid_x & 0x7);
            for (int mask = 1; mask < 0x7; mask *= 2)
            value += __shfl_xor(value, mask);
            size_t oi = BLOCK_DIM_X * _bid_x + _tid_x;
            out[oi] = value;

        }
    }
}
extern "C"

void bcast_shfl_sg16(const int arg, int *out) {
    #pragma HLS INTERFACE s_axilite port=arg
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int value = ((_tid_x & 0xf) == 0) ? arg : 0;
            // Synchronize all threads in warp, and get "value" from lane 0
            int out_v = __shfl( value, 0);
            size_t oi = BLOCK_DIM_X * _bid_x + _tid_x;
            out[oi] = out_v;

        }
    }
}
extern "C"

void bcast_shfl_xor_sg16(int *out) {
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int value = (_tid_x & 0xf);
            for (int mask = 1; mask < 0xf; mask *= 2)
            value += __shfl_xor(value, mask);
            size_t oi = BLOCK_DIM_X * _bid_x + _tid_x;
            out[oi] = value;

        }
    }
}
extern "C"

void bcast_shfl_sg32(const int arg, int *out) {
    #pragma HLS INTERFACE s_axilite port=arg
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int value = ((_tid_x & 0x1f) == 0) ? arg : 0;
            // Synchronize all threads in warp, and get "value" from lane 0
            int out_v = __shfl( value, 0);
            size_t oi = BLOCK_DIM_X * _bid_x + _tid_x;
            out[oi] = out_v;

        }
    }
}
extern "C"

void bcast_shfl_xor_sg32(int *out) {
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int value = (_tid_x & 0x1f);
            for (int mask = 1; mask < 0x1f; mask *= 2)
            value += __shfl_xor(value, mask);
            size_t oi = BLOCK_DIM_X * _bid_x + _tid_x;
            out[oi] = value;

        }
    }
}
extern "C"

void transpose_shfl(float* out, const float* in) {
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned b_start = BLOCK_DIM_X * _bid_x;
            unsigned b_offs = b_start + _tid_x;
            unsigned s_offs = BLOCK_DIM_X - _tid_x - 1;
            float val = in[b_offs];
            out[b_offs] = __shfl(val, s_offs);

        }
    }
}
