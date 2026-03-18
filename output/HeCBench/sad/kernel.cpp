#include "kernel.h"

// --- from main.cu ---
extern "C"
void compute_sad_array(
                    int* sad_array,
    const unsigned char* image,
    const unsigned char* kernel,
    const int sad_array_size,
    const int image_width,
    const int image_height,
    const int kernel_width,
    const int kernel_height,
    const int kernel_size)
{
    #pragma HLS INTERFACE m_axi port=sad_array offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=image offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=kernel offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=sad_array_size
    #pragma HLS INTERFACE s_axilite port=image_width
    #pragma HLS INTERFACE s_axilite port=image_height
    #pragma HLS INTERFACE s_axilite port=kernel_width
    #pragma HLS INTERFACE s_axilite port=kernel_height
    #pragma HLS INTERFACE s_axilite port=kernel_size
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int col = _bid_x * BLOCK_DIM_X + _tid_x;
                    int row = _bid_y * BLOCK_DIM_Y + _tid_y;
                    int sad_result = 0;

                    if (row < image_height && col < image_width) {
                    const int overlap_width = min(image_width - col, kernel_width);
                    const int overlap_height = min(image_height - row, kernel_height);
                    #pragma unroll 4
                    for (int kr = 0; kr < overlap_height; kr++) {
                    #pragma unroll 4
                    for (int kc = 0; kc < overlap_width; kc++) {
                    const int image_addr = ((row + kr) * image_width + (col + kc)) * 3;
                    const int kernel_addr = (kr * kernel_width + kc) * 3;
                    const int m_r = (int)(image[image_addr + 0]);
                    const int m_g = (int)(image[image_addr + 1]);
                    const int m_b = (int)(image[image_addr + 2]);
                    const int t_r = (int)(kernel[kernel_addr + 0]);
                    const int t_g = (int)(kernel[kernel_addr + 1]);
                    const int t_b = (int)(kernel[kernel_addr + 2]);
                    const int error = abs(m_r - t_r) + abs(m_g - t_g) + abs(m_b - t_b);
                    sad_result += error;
                    }
                    }

                    int norm_sad = (int)(sad_result / (float)kernel_size);

                    int my_index_in_sad_array = row * image_width + col;
                    if (my_index_in_sad_array < sad_array_size) {
                    sad_array[my_index_in_sad_array] = norm_sad;
                    }
                    }

                }
            }
        }
    }
}
extern "C"

void find_min_in_sad_array(
    const int sad_array_size,
    const int*  sad_array,
          int*  min_sad)
{
    #pragma HLS INTERFACE s_axilite port=sad_array_size
    #pragma HLS INTERFACE m_axi port=sad_array offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=min_sad offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=cache complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    unsigned int gid = _bid_x * BLOCK_DIM_X + _tid_x;
                    unsigned int stride = GRID_DIM_X * BLOCK_DIM_X;
                    unsigned int offset = 0;

                    int cache[BLOCK_SIZE];

                    int temp = FOUND_MIN;
                    while (gid + offset < sad_array_size) {
                    temp = min(temp, sad_array[gid + offset]);
                    offset += stride;
                    }

                    cache[_tid_x] = temp;

                    unsigned int i = BLOCK_DIM_X / 2;
                    while (i != 0) {
                    if (_tid_x < i)
                    cache[_tid_x] = min(cache[_tid_x], cache[_tid_x + i]);
                    i /= 2;
                    }

                    // Update global min for each block
                    if (_tid_x == 0)
                    (*min_sad = min(*min_sad, cache[0]));

                }
            }
        }
    }
}
extern "C"

void get_num_of_occurrences(
    const int sad_array_size,
    const int* sad_array,
    const int* min_sad,
          int* num_occurrences)
{
    #pragma HLS INTERFACE s_axilite port=sad_array_size
    #pragma HLS INTERFACE m_axi port=sad_array offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=min_sad offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=num_occurrences offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    unsigned int gid = _tid_x + _bid_x * BLOCK_DIM_X;

                    int s;

                    if (gid < sad_array_size) {

                    if (_tid_x == 0) s = 0;

                    if (sad_array[gid] == *min_sad)
                    (s += 1);

                    // Update global occurance for each block
                    if (_tid_x == 0)
                    (*num_occurrences += s);
                    }

                }
            }
        }
    }
}
