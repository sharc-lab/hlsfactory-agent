#include "kernel.h"

// --- from main.cu ---
extern "C"
void softMax (const int numSlice, const int sliceSize,
              const float* src, float* dest)
{
    #pragma HLS INTERFACE s_axilite port=numSlice
    #pragma HLS INTERFACE s_axilite port=sliceSize
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dest offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i >= numSlice) return;
            float max_ = src[i * sliceSize];
            for (int j = 0; j < sliceSize; j++) {
            max_ = max(max_, src[i * sliceSize + j]);
            }
            float sum = 0;
            for (int j = 0; j < sliceSize; j++) {
            sum += expf(src[i * sliceSize + j] - max_);
            }
            for (int j = 0; j < sliceSize; j++) {
            dest[i * sliceSize + j] = expf(src[i * sliceSize + j] - max_) / sum;
            }

        }
    }
}
extern "C"

void softMax2 (const int numSlice, const int sliceSize,
              const float* src, float* dest)
{
    #pragma HLS INTERFACE s_axilite port=numSlice
    #pragma HLS INTERFACE s_axilite port=sliceSize
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dest offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            #if defined(__GFX8__) || defined(__GFX9__)
            #define WarpSize 64
            #else
            #define WarpSize 32
            #endif

            int i = _bid_x * warp.meta_group_size() + warp.meta_group_rank();
            if (i >= numSlice) return;
            float max_ = src[i * sliceSize];
            for (int j = warp.thread_rank(); j < sliceSize; j += warp.size()) {
            max_ = max(max_, src[i * sliceSize + j]);
            }
            for (int offset = WarpSize/2; offset > 0; offset /= 2) {
            max_ = max(max_, warp.shfl_xor(max_, offset));
            }
            float sum = 0;
            for (int j = warp.thread_rank(); j < sliceSize; j += warp.size()) {
            sum += expf(src[i * sliceSize + j] - max_);
            }
            for (int offset = WarpSize/2; offset > 0; offset /= 2) {
            sum += warp.shfl_xor(sum, offset);
            }
            for (int j = warp.thread_rank(); j < sliceSize; j += warp.size()) {
            dest[i * sliceSize + j] = expf(src[i * sliceSize + j] - max_) / sum;
            }

        }
    }
}
