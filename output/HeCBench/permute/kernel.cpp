#include "kernel.h"

// --- from main.cu ---
extern "C"
void permute_kernel(
    float*  Q,
    float*  K,
    float*  V,
    const float* inp,
    int B, int T, int NH, int d) {
    #pragma HLS INTERFACE m_axi port=Q offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=K offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=V offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=inp offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=B
    #pragma HLS INTERFACE s_axilite port=T
    #pragma HLS INTERFACE s_axilite port=NH
    #pragma HLS INTERFACE s_axilite port=d
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            // okay so now, this kernel wants Q,K,V to all be of shape (B, NH, T, d)
            // but instead, we have a single tensor QKV (inp) of shape (B, T, 3, NH, d)
            int idx = _bid_x * BLOCK_DIM_X + _tid_x;

            // Q[b][nh_][n][d_] = inp[b][n][0][nh_][d_]
            int C = NH * d;

            if (idx < B * C * T) {
            int b = idx / (C * T);
            int rest = idx % (C * T);
            int nh_ = rest / (T * d);
            rest = rest % (T * d);
            int n = rest / d;
            int d_ = rest % d;

            int inp_idx = \
            (b * T * 3 * C)
            +   (n * 3 * C)
            +       (0 * C)
            +          (nh_ * d)
            +                d_;

            Q[idx] = inp[inp_idx];
            K[idx] = inp[inp_idx + C];
            V[idx] = inp[inp_idx + 2 * C];
            }

        }
    }
}
