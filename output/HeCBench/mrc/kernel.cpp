#include "kernel.h"

// --- from main.cu ---
extern "C"
void MRCGradient (
    const int N, const int* Y, const float* X1, const float* X2, const float* dOutput,
    const float margin, float* dX1, float* dX2)
{
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE m_axi port=Y offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=X1 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=X2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=dOutput offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=margin
    #pragma HLS INTERFACE m_axi port=dX1 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=dX2 offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i < N) {
            float dist = -Y[i] * (X1[i] - X2[i]) + margin;
            if (dist < 0.f) {
            dX1[i] = dX2[i] = 0.f;
            } else {
            dX1[i] = -Y[i] * dOutput[i];
            dX2[i] = Y[i] * dOutput[i];
            }
            }

        }
    }
}
extern "C"

void MRCGradient2(
    const int N, const int* Y, const float* X1, const float* X2, const float* dOutput,
    const float margin, float* dX1, float* dX2)
{
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE m_axi port=Y offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=X1 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=X2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=dOutput offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=margin
    #pragma HLS INTERFACE m_axi port=dX1 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=dX2 offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i < N) {
            float y = Y[i];
            float o = dOutput[i];
            float dist = -y * (X1[i] - X2[i]) + margin;
            dX1[i] = dist < 0.f ? 0.f : -y * o;
            dX2[i] = dist < 0.f ? 0.f : y * o;
            }

        }
    }
}
