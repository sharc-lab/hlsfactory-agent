#include "kernel.h"

// --- from main.cu ---
extern "C"
void DetectionOverlayBox(
  const T* input,
        T*  output,
  int imgWidth, int imgHeight,
  int x0, int y0, int boxWidth, int boxHeight,
  const float4 color) 
{
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=imgWidth
    #pragma HLS INTERFACE s_axilite port=imgHeight
    #pragma HLS INTERFACE s_axilite port=x0
    #pragma HLS INTERFACE s_axilite port=y0
    #pragma HLS INTERFACE s_axilite port=boxWidth
    #pragma HLS INTERFACE s_axilite port=boxHeight
    #pragma HLS INTERFACE s_axilite port=color
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    const int box_x = _bid_x * BLOCK_DIM_X + _tid_x;
                    const int box_y = _bid_y * BLOCK_DIM_Y + _tid_y;

                    if( box_x >= boxWidth || box_y >= boxHeight ) return;

                    const int x = box_x + x0;
                    const int y = box_y + y0;

                    if( x >= imgWidth || y >= imgHeight ) return;

                    T px = input[ y * imgWidth + x ];

                    const float alpha = color.w / 255.0f;
                    const float ialph = 1.0f - alpha;

                    px.x = alpha * color.x + ialph * px.x;
                    px.y = alpha * color.y + ialph * px.y;
                    px.z = alpha * color.z + ialph * px.z;

                    output[y * imgWidth + x] = px;

                }
            }
        }
    }
}
