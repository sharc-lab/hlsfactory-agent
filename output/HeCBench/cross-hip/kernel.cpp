#include "kernel.h"

// --- from main.cu ---
extern "C"
void cross_kernel(
    int numel,
          T* out,
    const T* x1,
    const T* x2,
    StrideType ostride,
    StrideType x1stride,
    StrideType x2stride)
{
    #pragma HLS INTERFACE s_axilite port=numel
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=x1 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=x2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=ostride
    #pragma HLS INTERFACE s_axilite port=x1stride
    #pragma HLS INTERFACE s_axilite port=x2stride
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            for (int i = _bid_x * BLOCK_DIM_X + _tid_x;
            i < numel; i += BLOCK_DIM_X * GRID_DIM_X) {

            auto* out_row = out + 3*i;
            const auto* x1_row = x1 + 3*i;
            const auto* x2_row = x2 + 3*i;

            const T val0 = (x1_row[1 * x1stride] * x2_row[2 * x2stride] -
            x1_row[2 * x1stride] * x2_row[1 * x2stride]);

            const T val1 = (x1_row[2 * x1stride] * x2_row[0 * x2stride] -
            x1_row[0 * x1stride] * x2_row[2 * x2stride]);

            const T val2 = (x1_row[0 * x1stride] * x2_row[1 * x2stride] -
            x1_row[1 * x1stride] * x2_row[0 * x2stride]);

            out_row[0 * ostride] = val0;
            out_row[1 * ostride] = val1;
            out_row[2 * ostride] = val2;
            }

        }
    }
}
extern "C"

void cross2_kernel(
    int numel,
          T* out,
    const T* x1,
    const T* x2,
    StrideType ostride,
    StrideType x1stride,
    StrideType x2stride)
{
    #pragma HLS INTERFACE s_axilite port=numel
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=x1 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=x2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=ostride
    #pragma HLS INTERFACE s_axilite port=x1stride
    #pragma HLS INTERFACE s_axilite port=x2stride
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            for (int i = _bid_x * BLOCK_DIM_X + _tid_x;
            i < numel; i += BLOCK_DIM_X * GRID_DIM_X) {

            auto* out_row = out + 3*i;
            const auto* x1_row = x1 + 3*i;
            const auto* x2_row = x2 + 3*i;

            const T x1_c0 = x1_row[0 * x1stride];
            const T x1_c1 = x1_row[1 * x1stride];
            const T x1_c2 = x1_row[2 * x1stride];
            const T x2_c0 = x2_row[0 * x2stride];
            const T x2_c1 = x2_row[1 * x2stride];
            const T x2_c2 = x2_row[2 * x2stride];

            const T val0 = x1_c1 * x2_c2 - x1_c2 * x2_c1 ;

            const T val1 = x1_c2 * x2_c0 - x1_c0 * x2_c2 ;

            const T val2 = x1_c0 * x2_c1 - x1_c1 * x2_c0 ;

            out_row[0 * ostride] = val0;
            out_row[1 * ostride] = val1;
            out_row[2 * ostride] = val2;
            }

        }
    }
}
extern "C"

void cross3_kernel(
    int numel,
          T* out,
    const T* x1,
    const T* x2)
{
    #pragma HLS INTERFACE s_axilite port=numel
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=x1 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=x2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            for (int i = _bid_x * BLOCK_DIM_X + _tid_x;
            i < numel; i += BLOCK_DIM_X * GRID_DIM_X) {

            auto* out_row = out + 3*i;
            const auto* x1_row = x1 + 3*i;
            const auto* x2_row = x2 + 3*i;

            const T x1_c0 = x1_row[0];
            const T x1_c1 = x1_row[1];
            const T x1_c2 = x1_row[2];
            const T x2_c0 = x2_row[0];
            const T x2_c1 = x2_row[1];
            const T x2_c2 = x2_row[2];

            const T val0 = x1_c1 * x2_c2 - x1_c2 * x2_c1 ;

            const T val1 = x1_c2 * x2_c0 - x1_c0 * x2_c2 ;

            const T val2 = x1_c0 * x2_c1 - x1_c1 * x2_c0 ;

            out_row[0] = val0;
            out_row[1] = val1;
            out_row[2] = val2;
            }

        }
    }
}
