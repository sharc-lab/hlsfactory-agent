#include "kernel.h"

// --- from main.cu ---
extern "C"
void generate_nms_bitmap(const float4* rects, unsigned char* nmsbitmap, const float othreshold)
{
    #pragma HLS INTERFACE m_axi port=rects offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=nmsbitmap offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=othreshold
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    const int i = _bid_x * BLOCK_DIM_X + _tid_x;
                    const int j = _bid_y * BLOCK_DIM_Y + _tid_y;

                    if(rects[i].w < rects[j].w)
                    {
                    float area = (rects[j].z + 1.0f) * (rects[j].z + 1.0f);
                    float w = fmaxf(0.0f, fminf(rects[i].x + rects[i].z, rects[j].x + rects[j].z) - fmaxf(rects[i].x, rects[j].x) + 1.0f);
                    float h = fmaxf(0.0f, fminf(rects[i].y + rects[i].z, rects[j].y + rects[j].z) - fmaxf(rects[i].y, rects[j].y) + 1.0f);
                    nmsbitmap[i * MAX_DETECTIONS + j] = (((w * h) / area) < othreshold) && (rects[j].z != 0);
                    }

                }
            }
        }
    }
}

void compute_nms_point_mask(unsigned char* pointsbitmap, int cond, int idx, int ndetections)
{
  *pointsbitmap = __syncthreads_and(cond);
}
extern "C"

void reduce_nms_bitmap(unsigned char* nmsbitmap, unsigned char* pointsbitmap, int ndetections)
{
    #pragma HLS INTERFACE m_axi port=nmsbitmap offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=pointsbitmap offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=ndetections
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int idx = _bid_x * MAX_DETECTIONS + _tid_x;

                    compute_nms_point_mask(&pointsbitmap[_bid_x], nmsbitmap[idx], idx, ndetections);

                    for(int i=0; i<(N_PARTITIONS-1); i++)
                    {
                    idx += MAX_DETECTIONS / N_PARTITIONS;
                    compute_nms_point_mask(&pointsbitmap[_bid_x], pointsbitmap[_bid_x] && nmsbitmap[idx], idx, ndetections);
                    }

                }
            }
        }
    }
} 
