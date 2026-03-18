#include "kernel.h"

// --- from main.cu ---
extern "C"
void gelu_bias_loop(__half* src, const __half* bias, int width, int height)
{
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=bias offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                int x     = _bid_x;  // seq length
                int y     = _tid_x * 2;
                int batch = _bid_y;

                int    index = batch * width * height + x * width;
                for (; y < width; y = y + BLOCK_DIM_X * 2) {
                auto v_bias = ((half2*)bias)[y >> 1];
                auto v_src  = ((half2*)src)[(index + y) >> 1];
                auto v      = __hadd2(v_src, v_bias);
                auto t      = __half22float2(v);
                t.x    = (0.5f * t.x * (1.0f + tanhf(0.79788456f * (t.x + 0.044715f * t.x * t.x * t.x))));
                t.y    = (0.5f * t.y * (1.0f + tanhf(0.79788456f * (t.y + 0.044715f * t.y * t.y * t.y))));
                ((half2*)src)[(index + y) >> 1] = __float22half2_rn(t);
                }

            }
        }
    }
}
extern "C"

void gelu_bias_loop_base(__half* src, const __half* bias, int width, int height)
{
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=bias offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                int x     = _bid_x;  // seq length
                int batch = _bid_y;

                int   index = batch * width * height + x * width;
                for (int y = _tid_x; y < width; y = y + BLOCK_DIM_X) {
                auto v_bias = bias[y];
                auto v_src  = src[index + y];
                auto v      = v_src + v_bias;
                auto t      = __half2float(v);
                t      = (0.5f * t * (1.0f + tanhf(0.79788456f * (t + 0.044715f * t * t * t))));
                src[index + y] = __float2half_rn(t);
                }

            }
        }
    }
}
