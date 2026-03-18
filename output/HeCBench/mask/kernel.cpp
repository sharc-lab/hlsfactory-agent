#include "kernel.h"

// --- from main.cu ---
extern "C"
void sequenceMaskKernel(
    int N,
    int M,
    int B,
    const T* in,
    const int* seq_lengths,
    T fill_val,
    T* out)
{
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=B
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=seq_lengths offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=fill_val
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            if (B >= 0) {
            KERNEL_LOOP(index, B * N * M) {
            int k = index % M;
            int j = (index - k) / M % N;
            int i = (index - M * j - k) / (N * M);
            int ind = N * M * i + M * j + k;
            out[ind] = (k >= seq_lengths[j] ? fill_val : in[ind]);
            }
            } else {
            KERNEL_LOOP(index, N * M) {
            int i = index / M;
            int j = index % M;
            out[index] = (j >= seq_lengths[i] ? fill_val : in[index]);
            }
            }

        }
    }
}
extern "C"

void windowMaskKernel(
    int N,
    int M,
    int B,
    const T* in,
    const int* window_centers,
    const int radius,
    T fill_val,
    T* out) {
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=B
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=window_centers offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=radius
    #pragma HLS INTERFACE s_axilite port=fill_val
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            if (B >= 0) {
            KERNEL_LOOP(index, B * N * M) {
            int k = index % M;
            int j = (index - k) / M % N;
            int i = (index - M * j - k) / (N * M);

            int ind = N * M * i + M * j + k;
            out[ind] =
            (k < window_centers[j] - radius || k > window_centers[j] + radius
            ? fill_val
            : in[ind]);
            }
            } else {
            KERNEL_LOOP(index, N * M) {
            int i = index / M;
            int j = index % M;

            out[index] =
            (j < window_centers[i] - radius || j > window_centers[i] + radius
            ? fill_val
            : in[index]);
            }
            }

        }
    }
}
extern "C"

void upperMaskKernel(int N, int M, int B, const T* in, T fill_val, T* out) {
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=B
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=fill_val
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            if (B >= 0) {
            KERNEL_LOOP(index, B * N * M) {
            int k = index % M;
            int j = (index - k) / M % N;
            int i = (index - M * j - k) / (N * M);

            int ind = N * M * i + M * j + k;
            out[ind] = (k > j ? fill_val : in[ind]);
            }
            } else {
            KERNEL_LOOP(index, N * M) {
            int i = index / M;
            int j = index % M;

            out[index] = (j > i ? fill_val : in[index]);
            }
            }

        }
    }
}
extern "C"

void lowerMaskKernel(int N, int M, int B, const T* in, T fill_val, T* out) {
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=B
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=fill_val
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            if (B >= 0) {
            KERNEL_LOOP(index, B * N * M) {
            int k = index % M;
            int j = (index - k) / M % N;
            int i = (index - M * j - k) / (N * M);

            int ind = N * M * i + M * j + k;
            out[ind] = (k < j ? fill_val : in[ind]);
            }
            } else {
            KERNEL_LOOP(index, N * M) {
            int i = index / M;
            int j = index % M;

            out[index] = (j < i ? fill_val : in[index]);
            }
            }

        }
    }
}
extern "C"

void upperDiagMaskKernel(int N, int M, int B, const T* in, T fill_val, T* out) {
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=B
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=fill_val
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            if (B >= 0) {
            KERNEL_LOOP(index, B * N * M) {
            int k = index % M;
            int j = (index - k) / M % N;
            int i = (index - M * j - k) / (N * M);

            int ind = N * M * i + M * j + k;
            out[ind] = (k >= j ? fill_val : in[ind]);
            }
            } else {
            KERNEL_LOOP(index, N * M) {
            int i = index / M;
            int j = index % M;

            out[index] = (j >= i ? fill_val : in[index]);
            }
            }

        }
    }
}
extern "C"

void lowerDiagMaskKernel(int N, int M, int B, const T* in, T fill_val, T* out) {
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=B
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=fill_val
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            if (B >= 0) {
            KERNEL_LOOP(index, B * N * M) {
            int k = index % M;
            int j = (index - k) / M % N;
            int i = (index - M * j - k) / (N * M);

            int ind = N * M * i + M * j + k;
            out[ind] = (k <= j ? fill_val : in[ind]);
            }
            } else {
            KERNEL_LOOP(index, N * M) {
            int i = index / M;
            int j = index % M;

            out[index] = (j <= i ? fill_val : in[index]);
            }
            }

        }
    }
}
