#include "kernel.h"

// --- from main.cu ---
extern "C"
void entropy(
        float * d_entropy,
    const char* d_val, 
    int height, int width)
{
    #pragma HLS INTERFACE m_axi port=d_entropy offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_val offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    const int x = _tid_x + _bid_x * BLOCK_DIM_X;
                    const int y = _tid_y + _bid_y * BLOCK_DIM_Y;
                    if (y >= height || x >= width) return;

                    // value of matrix element ranges from 0 inclusive to 16 exclusive
                    char count[16];
                    for (int i = 0; i < 16; i++) count[i] = 0;

                    // total number of valid elements
                    char total = 0;

                    // 5x5 window
                    for(int dy = -2; dy <= 2; dy++) {
                    for(int dx = -2; dx <= 2; dx++) {
                    int xx = x + dx;
                    int yy = y + dy;
                    if(xx >= 0 && yy >= 0 && yy < height && xx < width) {
                    count[d_val[yy * width + xx]]++;
                    total++;
                    }
                    }
                    }

                    float entropy = 0;
                    if (total < 1) {
                    total = 1;
                    } else {
                    for(int k = 0; k < 16; k++) {
                    float p = __fdividef((float)count[k], (float)total);
                    entropy -= p * __log2f(p);
                    }
                    }

                    d_entropy[y * width + x] = entropy;

                }
            }
        }
    }
}
extern "C"

void entropy_opt(
       float * d_entropy,
  const  char* d_val, 
  const float* d_logTable,
  int height, int width)
{
    #pragma HLS INTERFACE m_axi port=d_entropy offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_val offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_logTable offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sd_count complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int sd_count[16][bsize_y*bsize_x];

                    const int x = _tid_x + _bid_x * BLOCK_DIM_X;
                    const int y = _tid_y + _bid_y * BLOCK_DIM_Y;
                    if (y >= height || x >= width) return;

                    const int idx = _tid_y*bsize_x + _tid_x;

                    for(int i = 0; i < 16;i++) sd_count[i][idx] = 0;

                    char total = 0;
                    for(int dy = -2; dy <= 2; dy++) {
                    for(int dx = -2; dx <= 2; dx++) {
                    int xx = x + dx,
                    yy = y + dy;

                    if(xx >= 0 && yy >= 0 && yy < height && xx < width) {
                    sd_count[d_val[yy*width+xx]][idx]++;
                    total++;
                    }
                    }
                    }

                    float entropy = 0;
                    for(int k = 0; k < 16; k++)
                    entropy -= d_logTable[sd_count[k][idx]];

                    entropy = entropy / total + __log2f(total);
                    d_entropy[y*width+x] = entropy;

                }
            }
        }
    }
}
