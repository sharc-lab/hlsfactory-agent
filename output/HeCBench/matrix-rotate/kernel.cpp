#include "kernel.h"

// --- from main.cu ---
extern "C"
void rotate_matrix_parallel (float *matrix, const int n) {
    #pragma HLS INTERFACE m_axi port=matrix offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int layer = _bid_x * BLOCK_DIM_X + _tid_x;
            if (layer < n/2) {
            int first = layer;
            int last = n - 1 - layer;
            for(int i = first; i < last; ++i) {
            int offset = i - first;

            float top = matrix[first*n+i]; // save top
            // left -> top
            matrix[first*n+i] = matrix[(last-offset)*n+first];

            // bottom -> left
            matrix[(last-offset)*n+first] = matrix[last*n+(last-offset)];

            // right -> bottom
            matrix[last*n+(last-offset)] = matrix[i*n+last];

            // top -> right
            matrix[i*n+last] = top; // right <- saved top
            }
            }

        }
    }
}
