#include "kernel.h"

// --- from haar.cu ---
extern "C"
void filter_kernel (const int* d_rectangles_array, 
                    int** d_scaled_rectangles_array, 
                    int* data, int width, int total_nodes)
{
    #pragma HLS INTERFACE m_axi port=d_rectangles_array offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_scaled_rectangles_array offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=total_nodes
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int gid = BLOCK_DIM_X * _bid_x + _tid_x;
            if (gid >= total_nodes) return;

            int idx = gid * 12;
            for (int k = 0; k < 3; k++)
            {
            int tr_x = d_rectangles_array[idx + k * 4];
            int tr_y = d_rectangles_array[idx + 1 + k * 4];
            int tr_width = d_rectangles_array[idx + 2 + k * 4];
            int tr_height = d_rectangles_array[idx + 3 + k * 4];
            int *p0 = data + width * (tr_y) + (tr_x);
            int *p1 = data + width * (tr_y) + (tr_x + tr_width);
            int *p2 = data + width * (tr_y + tr_height) + (tr_x);
            int *p3 = data + width * (tr_y + tr_height) + (tr_x + tr_width);
            if (k < 2)
            {
            d_scaled_rectangles_array[idx + k * 4]     = p0;
            d_scaled_rectangles_array[idx + k * 4 + 1] = p1;
            d_scaled_rectangles_array[idx + k * 4 + 2] = p2;
            d_scaled_rectangles_array[idx + k * 4 + 3] = p3;
            }
            else
            {
            bool z = ((tr_x == 0) && (tr_y == 0) && (tr_width == 0) && (tr_height == 0));
            d_scaled_rectangles_array[idx + k * 4]     = z ? NULL : p0;
            d_scaled_rectangles_array[idx + k * 4 + 1] = z ? NULL : p1;
            d_scaled_rectangles_array[idx + k * 4 + 2] = z ? NULL : p2;
            d_scaled_rectangles_array[idx + k * 4 + 3] = z ? NULL : p3;
            } /* end of branch if(k<2) */
            }   /* end of k loop */

        }
    }
}
