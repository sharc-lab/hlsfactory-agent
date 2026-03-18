#include "kernel.h"

// --- from gemv.cu ---
extern "C"
void check_correctness(__half* mat, __half* vec, __half* res, int n) {
    #pragma HLS INTERFACE m_axi port=mat offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=vec offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=res offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int idx = _bid_x * BLOCK_DIM_X + _tid_x;
                    if (idx < n) {
                    float result = 0;
                    for (int j = 0; j < n; ++j) {
                    result += __half2float(mat[idx * n + j]) * __half2float(vec[j]);
                    }
                    float diff = result - __half2float(res[idx]);
                    float delta = 0.125 * n / 512;
                    if (diff > delta || diff < -delta) {
                    printf("!!![idx=%d] %f != %f, diff=%f\n", idx, __half2float(res[idx]),
                    result, diff);
                    }
                    }

                }
            }
        }
    }
}
extern "C"

void check_int8_quantized_correctness(int8_t* mat, __half* vec,
                                                 __half* res, __half scale,
                                                 __half zero_point, int n) {
    #pragma HLS INTERFACE m_axi port=mat offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=vec offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=res offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=scale
    #pragma HLS INTERFACE s_axilite port=zero_point
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int idx = _bid_x * BLOCK_DIM_X + _tid_x;
                    if (idx < n) {
                    float result = 0;
                    for (int j = 0; j < n; ++j) {
                    float dequantized_val = (static_cast<float>(mat[idx * n + j]) -
                    static_cast<float>(zero_point)) *
                    static_cast<float>(scale);
                    result += dequantized_val * __half2float(vec[j]);
                    }
                    float diff = result - __half2float(res[idx]);
                    float delta = 0.125 * n / 512;
                    if (diff > delta || diff < -delta) {
                    printf("!!![idx=%d] %f != %f, diff=%f\n", idx, __half2float(res[idx]),
                    result, diff);
                    }
                    }

                }
            }
        }
    }
}
extern "C"

void check_int4_quantized_correctness(uint4_2* mat, __half* vec,
                                                 __half* res, __half scale,
                                                 __half zero_point,
                                                 int mat_size) {
    #pragma HLS INTERFACE m_axi port=mat offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=vec offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=res offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=scale
    #pragma HLS INTERFACE s_axilite port=zero_point
    #pragma HLS INTERFACE s_axilite port=mat_size
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int idx = _bid_x * BLOCK_DIM_X + _tid_x;
                    if (idx < mat_size * 2) {
                    float result = 0;
                    for (int j = 0; j < mat_size; ++j) {
                    uint8_t x = mat[idx * mat_size + j].getX();
                    uint8_t y = mat[idx * mat_size + j].getY();
                    float dequantized_x =
                    (static_cast<float>(x) - static_cast<float>(zero_point)) *
                    static_cast<float>(scale);
                    float dequantized_y =
                    (static_cast<float>(y) - static_cast<float>(zero_point)) *
                    static_cast<float>(scale);
                    result += dequantized_x * __half2float(vec[j * 2]);
                    result += dequantized_y * __half2float(vec[j * 2 + 1]);
                    }
                    float diff = result - __half2float(res[idx]);
                    float delta = 0.125 * mat_size / 256;
                    if (diff > delta || diff < -delta) {
                    printf("!!![idx=%d] %f != %f, diff=%f\n", idx, __half2float(res[idx]),
                    result, diff);
                    }
                    }

                }
            }
        }
    }
}
