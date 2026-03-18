#include "kernel.h"

// --- from main.cu ---
extern "C"
void base_elementwise_add_kernel(
    const __half*  gA,
    const __half*  gB,
          __half*  gC,
    const size_t n)
{
    #pragma HLS INTERFACE m_axi port=gA offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=gB offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=gC offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                size_t idx = (size_t)_bid_x * BLOCK_DIM_X + _tid_x;
                if (idx < n) gC[idx] = gA[idx] + gB[idx];

            }
        }
    }
}
extern "C"

void vectorized_elementwise_add_kernel(
    const __half*  gA,
    const __half*  gB,
          __half*  gC,
    const size_t n)
{
    #pragma HLS INTERFACE m_axi port=gA offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=gB offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=gC offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                const int block_work_size = BLOCK_DIM_X * vec_size;
                auto index = static_cast<size_t>(_bid_x) * block_work_size + _tid_x * vec_size;

                auto remaining = n - index;
                if (remaining < vec_size) {
                for (auto i = index; i < n; i++) {
                gC[i] = gA[i] + gB[i];
                }
                } else {

                auto a8 = *reinterpret_cast<const float4*>(gA + index);
                auto b8 = *reinterpret_cast<const float4*>(gB + index);

                auto a0 = *reinterpret_cast<__half2*>(&a8.x);
                auto a1 = *reinterpret_cast<__half2*>(&a8.y);
                auto a2 = *reinterpret_cast<__half2*>(&a8.z);
                auto a3 = *reinterpret_cast<__half2*>(&a8.w);
                auto b0 = *reinterpret_cast<__half2*>(&b8.x);
                auto b1 = *reinterpret_cast<__half2*>(&b8.y);
                auto b2 = *reinterpret_cast<__half2*>(&b8.z);
                auto b3 = *reinterpret_cast<__half2*>(&b8.w);

                float4 c8;
                *reinterpret_cast<__half2*>(&c8.x) = __hadd2(a0, b0);
                *reinterpret_cast<__half2*>(&c8.y) = __hadd2(a1, b1);
                *reinterpret_cast<__half2*>(&c8.z) = __hadd2(a2, b2);
                *reinterpret_cast<__half2*>(&c8.w) = __hadd2(a3, b3);
                *reinterpret_cast<float4*>(gC + index) = c8;
                }

            }
        }
    }
}
extern "C"

void tv_elementwise_add_kernel(
    const __half*  gA,
    const __half*  gB,
          __half*  gC,
    const int M,
    const int N)
{
    #pragma HLS INTERFACE m_axi port=gA offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=gB offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=gC offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                // Block-tile origin in global memory
                int blk_row = _bid_y * TILE_M;   // first row of this block's tile
                int blk_col = _bid_x * TILE_N;   // first col of this block's tile

                // Thread decomposition within the tile:
                int warp_id = _tid_x / WARP_SIZE;
                int lane_id = _tid_x % WARP_SIZE;

                // VALS_M rows for each warp
                int row_start = blk_row + warp_id * VALS_M;

                // A 128-bit (8×fp16) load for each thread
                int col_start = blk_col + lane_id * VALS_N;

                __half a_frag[VALS_M][VALS_N];
                __half b_frag[VALS_M][VALS_N];
                __half c_frag[VALS_M][VALS_N];

                // merge three loops: load, compute, store
                #pragma unroll
                for (int vm = 0; vm < VALS_M; ++vm) {
                int row = row_start + vm;
                // 128-bit (float4 = 8×fp16) vectorised load
                size_t base = (size_t)row * N + col_start;
                *reinterpret_cast<float4*>(a_frag[vm]) = *reinterpret_cast<const float4*>(gA + base);
                *reinterpret_cast<float4*>(b_frag[vm]) = *reinterpret_cast<const float4*>(gB + base);

                auto a = reinterpret_cast<const __half2*>(a_frag[vm]);
                auto b = reinterpret_cast<const __half2*>(b_frag[vm]);
                auto c = reinterpret_cast<__half2*>(c_frag[vm]);

                #pragma unroll
                for (int i = 0; i < VALS_N / 2; i++)
                c[i] = __hadd2(a[i], b[i]);

                *reinterpret_cast<float4*>(gC + base) = *reinterpret_cast<const float4*>(c_frag[vm]);
                }

            }
        }
    }
}
