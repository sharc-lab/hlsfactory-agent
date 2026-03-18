#include "kernel.h"

// --- from main.cu ---
extern "C"
void k0 (const float * a, float * o) {
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=o offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int t = _bid_x * BLOCK_DIM_X + _tid_x;
            float x = a[t];
            o[t] = coshf(x)/sinhf(x) - 1.f/x;

        }
    }
}
extern "C"

void k1 (const float * a, float * o) {
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=o offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int t = _bid_x * BLOCK_DIM_X + _tid_x;
            float x = a[t];
            o[t] = 1.f / tanhf(x) - 1.f/x;

        }
    }
}
extern "C"

void k2 (const float * a, float * o) {
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=o offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int t = _bid_x * BLOCK_DIM_X + _tid_x;
            float x = a[t];
            float s, r;
            s = x * x;
            r =              7.70960469e-8f;
            r = fmaf (r, s, -1.65101926e-6f);
            r = fmaf (r, s,  2.03457112e-5f);
            r = fmaf (r, s, -2.10521728e-4f);
            r = fmaf (r, s,  2.11580913e-3f);
            r = fmaf (r, s, -2.22220998e-2f);
            r = fmaf (r, s,  8.33333284e-2f);
            r = fmaf (r, x,  0.25f * x);
            o[t] = r;

        }
    }
}
