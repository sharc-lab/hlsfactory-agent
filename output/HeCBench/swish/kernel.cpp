#include "kernel.h"

// --- from main.cu ---
extern "C"
void SwishKernel(const int N, const T* X, T* Y)
{
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE m_axi port=X offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=Y offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            KERNEL_LOOP(i, N) {
            Y[i] = __ldg(X + i) / (T(1) + exp(-__ldg(X + i)));
            }

        }
    }
}
extern "C"

void SwishGradientKernel(
    const int N,
    const T* X,
    const T* Y,
    const T* dY,
          T* dX)
{
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE m_axi port=X offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=Y offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dY offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=dX offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            KERNEL_LOOP(i, N) {
            dX[i] = __ldg(dY + i) *
            (__ldg(Y + i) + (T(1) - __ldg(Y + i)) / (T(1) + exp(-__ldg(X + i))));
            }

        }
    }
}
