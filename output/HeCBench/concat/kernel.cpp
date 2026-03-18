#include "kernel.h"

// --- from main.cu ---
extern "C"
void concat (const T * inp1,
             const T * inp2,
                   T *output,
             int sz0, int sz2, int sz1_1, int sz1_2)
{
    #pragma HLS INTERFACE m_axi port=inp1 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=inp2 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=sz0
    #pragma HLS INTERFACE s_axilite port=sz2
    #pragma HLS INTERFACE s_axilite port=sz1_1
    #pragma HLS INTERFACE s_axilite port=sz1_2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int nele = sz0 * sz2 * (sz1_1 + sz1_2);
            int idx = _bid_x * BLOCK_DIM_X + _tid_x;
            if (idx >= nele) return;

            float *dst_ptr = (float *)output + idx;
            int idx2 = idx % sz2;
            idx = idx / sz2;
            int idx1 = idx % (sz1_1 + sz1_2);
            int idx0 = idx / (sz1_1 + sz1_2);
            float *src_ptr;
            int sz1;
            if (idx1 < sz1_1) {
            sz1 = sz1_1;
            src_ptr = (float *)inp1;
            } else {
            idx1 -= sz1_1;
            sz1 = sz1_2;
            src_ptr = (float *)inp2;
            }
            src_ptr += flat_3dim(idx0, idx1, idx2, sz1, sz2);
            *dst_ptr = *src_ptr;

        }
    }
}
