#include "kernel.h"

// --- from main.cu ---
extern "C"
void ChannelShuffleNCHWKernel(
    const int G,
    const int K,
    const int HxW,
    const T* X,
          T* Y)
{
    #pragma HLS INTERFACE s_axilite port=G
    #pragma HLS INTERFACE s_axilite port=K
    #pragma HLS INTERFACE s_axilite port=HxW
    #pragma HLS INTERFACE m_axi port=X offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=Y offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    const int C = G * K;
                    const int n = kNFirst ? _bid_x : _bid_y;
                    const int s = kNFirst ? _bid_y : _bid_x;
                    const int g = _bid_z % G;
                    const int k = _bid_z / G;
                    const int offset = s * NUM_THREADS + _tid_x;
                    if (offset < HxW) {
                    Y[(n * C + _bid_z) * HxW + offset] =
                    __ldg(X + (n * C + g * K + k) * HxW + offset);
                    }

                }
            }
        }
    }
}
extern "C"

void ChannelShuffleNHWCKernel(const int G, const int K, const T* X, T* Y)
{
    #pragma HLS INTERFACE s_axilite port=G
    #pragma HLS INTERFACE s_axilite port=K
    #pragma HLS INTERFACE m_axi port=X offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=Y offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sdata complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    T sdata[kSharedSize];
                    const int C = G * K;
                    const int offset = _bid_x * C;
                    for (int i = _tid_x; i < C; i += BLOCK_DIM_X) {
                    sdata[i] = __ldg(X + offset + i);
                    }
                    for (int i = _tid_x; i < C; i += BLOCK_DIM_X) {
                    const int g = i % G;
                    const int k = i / G;
                    Y[offset + i] = sdata[g * K + k];
                    }

                }
            }
        }
    }
}
