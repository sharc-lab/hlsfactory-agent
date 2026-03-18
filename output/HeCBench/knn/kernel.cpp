#include "kernel.h"

// --- from main.cu ---
extern "C"
void cuInsertionSort(float * dist,
                       int * ind,
                       int width, int height, int k)
{
    #pragma HLS INTERFACE m_axi port=dist offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=ind offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    // Variables
                    int l, i, j;
                    float *p_dist;
                    int *p_ind;
                    float curr_dist, max_dist;
                    int curr_row, max_row;
                    unsigned int xIndex = _bid_x * BLOCK_DIM_X + _tid_x;

                    if (xIndex < width) {
                    // Pointer shift, initialization, and max value
                    p_dist = dist + xIndex;
                    p_ind = ind + xIndex;
                    max_dist = p_dist[0];
                    p_ind[0] = 0;

                    // Part 1 : sort kth firt elementZ
                    for (l = 1; l < k; l++) {
                    curr_row = l * width;
                    curr_dist = p_dist[curr_row];
                    if (curr_dist < max_dist) {
                    i = l - 1;
                    for (int a = 0; a < l - 1; a++) {
                    if (p_dist[a * width] > curr_dist) {
                    i = a;
                    break;
                    }
                    }
                    for (j = l; j > i; j--) {
                    p_dist[j * width] = p_dist[(j - 1) * width];
                    p_ind[j * width] = p_ind[(j - 1) * width];
                    }
                    p_dist[i * width] = curr_dist;
                    p_ind[i * width] = l;
                    } else {
                    p_ind[l * width] = l;
                    }
                    max_dist = p_dist[curr_row];
                    }

                    // Part 2 : insert element in the k-th first lines
                    max_row = (k - 1) * width;
                    for (l = k; l < height; l++) {
                    curr_dist = p_dist[l * width];
                    if (curr_dist < max_dist) {
                    i = k - 1;
                    for (int a = 0; a < k - 1; a++) {
                    if (p_dist[a * width] > curr_dist) {
                    i = a;
                    break;
                    }
                    }
                    for (j = k - 1; j > i; j--) {
                    p_dist[j * width] = p_dist[(j - 1) * width];
                    p_ind[j * width] = p_ind[(j - 1) * width];
                    }
                    p_dist[i * width] = curr_dist;
                    p_ind[i * width] = l;
                    max_dist = p_dist[max_row];
                    }
                    }
                    }

                }
            }
        }
    }
}
