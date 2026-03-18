#include "kernel.h"

// --- from main.cu ---
extern "C"
void conv1d(const T *  in,
                  T *  out,
            const int input_width,
            const int mask_width)
{
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=input_width
    #pragma HLS INTERFACE s_axilite port=mask_width
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _tid_x + _bid_x * BLOCK_DIM_X;
            T s = 0;
            int start = i - mask_width / 2;
            for (int j = 0; j < mask_width; j++) {
            if (start + j >= 0 && start + j < input_width) {
            s += in[start + j] * mask<T>[j];
            }
            }
            out[i] = s;

        }
    }
}
extern "C"

void conv1d_tiled(const T * in,
                        T * out,
                  const int input_width,
                  const int mask_width)
{
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=input_width
    #pragma HLS INTERFACE s_axilite port=mask_width
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=smem complete dim=1
    #pragma HLS ARRAY_PARTITION variable=smem complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned char smem[4096]; // TILE_SIZE + MAX_MASK_WIDTH - 1;
            T *tile = reinterpret_cast<T*>(smem);
            int i = _tid_x + _bid_x * BLOCK_DIM_X;

            int n = mask_width / 2;  // last n cells of the previous tile

            // load left cells
            int halo_left = (_bid_x - 1) * BLOCK_DIM_X + _tid_x;
            if (_tid_x >= BLOCK_DIM_X - n)
            tile[_tid_x - (BLOCK_DIM_X - n)] = halo_left < 0 ? 0 : in[halo_left];

            // load center cells
            tile[n + _tid_x] = in[_bid_x * BLOCK_DIM_X + _tid_x];

            // load right cells
            int halo_right = (_bid_x + 1) * BLOCK_DIM_X + _tid_x;
            if (_tid_x < n)
            tile[_tid_x + BLOCK_DIM_X + n] = halo_right >= input_width ? 0 : in[halo_right];

            T s = 0;
            for (int j = 0; j < mask_width; j++)
            s += tile[_tid_x + j] * mask<T>[j];

            out[i] = s;

        }
    }
}
extern "C"

void conv1d_tiled_caching(const T * in,
                                T * out,
                          const int input_width,
                          const int mask_width)
{
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=input_width
    #pragma HLS INTERFACE s_axilite port=mask_width
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=smem complete dim=1
    #pragma HLS ARRAY_PARTITION variable=smem complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned char smem[4096]; // TILE_SIZE
            T *tile = reinterpret_cast<T*>(smem);

            int i = _tid_x + _bid_x * BLOCK_DIM_X;
            tile[_tid_x] = in[i];

            int this_tile_start = _bid_x * BLOCK_DIM_X;
            int next_tile_start = (_bid_x + 1) * BLOCK_DIM_X;
            int start = i - (mask_width / 2);
            T s = 0;
            for (int j = 0; j < mask_width; j++) {
            int in_index = start + j;
            if (in_index >= 0 && in_index < input_width) {
            if (in_index >= this_tile_start && in_index < next_tile_start) {
            // in_index = (start + j) = (i - mask_width/2 +j) >= 0,
            // then map in_index to tile_index
            s += tile[_tid_x + j - (mask_width / 2)] * mask<T>[j];
            } else {
            s += in[in_index] * mask<T>[j];
            }
            }
            }
            out[i] = s;

        }
    }
}
