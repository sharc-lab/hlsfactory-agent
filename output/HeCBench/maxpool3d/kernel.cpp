#include "kernel.h"

// --- from main.cu ---
extern "C"
void maxpool3d(
  const DTYPE* i_img,
        DTYPE* o_img,
  const int Hstride,
  const int Vstride,
  const int pool_width,
  const int pool_height,
  const int i_img_count,
  const int i_img_width,
  const int i_img_height,
  const int o_img_width,
  const int o_img_height )
{
    #pragma HLS INTERFACE m_axi port=i_img offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=o_img offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=Hstride
    #pragma HLS INTERFACE s_axilite port=Vstride
    #pragma HLS INTERFACE s_axilite port=pool_width
    #pragma HLS INTERFACE s_axilite port=pool_height
    #pragma HLS INTERFACE s_axilite port=i_img_count
    #pragma HLS INTERFACE s_axilite port=i_img_width
    #pragma HLS INTERFACE s_axilite port=i_img_height
    #pragma HLS INTERFACE s_axilite port=o_img_width
    #pragma HLS INTERFACE s_axilite port=o_img_height
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            const int x = _bid_x * BLOCK_DIM_X + _tid_x;
                            const int y = _bid_y * BLOCK_DIM_Y + _tid_y;
                            const int z = _bid_z * BLOCK_DIM_Z + _tid_z;
                            if (x >= o_img_width || y >= o_img_height || z >= i_img_count)
                            return;

                            const int xidx = Hstride * x;
                            const int yidx = Vstride * y;
                            DTYPE maxval = (DTYPE)0;

                            for (int r = 0; r < pool_height; r++)
                            {
                            const int idxIntmp = ((z * i_img_height + yidx + r) * i_img_width) + xidx;
                            for(int c = 0; c < pool_width; c++)
                            {
                            const int idxIn = idxIntmp + c;
                            maxval = fmaxf(maxval, i_img[idxIn]);
                            }
                            }
                            o_img[(((z * o_img_height) + y) * o_img_width) + x] = maxval;

                        }
                    }
                }
            }
        }
    }
}
