#include "kernel.h"

// --- from conv.cu ---
extern "C"
void conv_rows(
    float * dst,
    const float * src,
    const float * kernel,
    const int imageW,
    const int imageH,
    const int pitch)
{
    #pragma HLS INTERFACE m_axi port=dst offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=kernel offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=imageW
    #pragma HLS INTERFACE s_axilite port=imageH
    #pragma HLS INTERFACE s_axilite port=pitch
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=l_Data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=l_Data complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float l_Data[ROWS_BLOCKDIM_Y][(ROWS_RESULT_STEPS + 2 * ROWS_HALO_STEPS) * ROWS_BLOCKDIM_X];

                    int gidX = _bid_x;
                    int gidY = _bid_y;
                    int lidX = _tid_x;
                    int lidY = _tid_y;
                    //Offset to the left halo edge
                    const int baseX = (gidX * ROWS_RESULT_STEPS - ROWS_HALO_STEPS) * ROWS_BLOCKDIM_X + lidX;
                    const int baseY = gidY * ROWS_BLOCKDIM_Y + lidY;

                    src += baseY * pitch + baseX;
                    dst += baseY * pitch + baseX;

                    //Load main data
                    #pragma unroll
                    for(int i = ROWS_HALO_STEPS; i < ROWS_HALO_STEPS + ROWS_RESULT_STEPS; i++)
                    l_Data[lidY][lidX + i * ROWS_BLOCKDIM_X] = src[i * ROWS_BLOCKDIM_X];

                    //Load left halo
                    #pragma unroll
                    for(int i = 0; i < ROWS_HALO_STEPS; i++)
                    l_Data[lidY][lidX + i * ROWS_BLOCKDIM_X] = (baseX + i * ROWS_BLOCKDIM_X >= 0) ? src[i * ROWS_BLOCKDIM_X] : 0;

                    //Load right halo
                    #pragma unroll
                    for(int i = ROWS_HALO_STEPS + ROWS_RESULT_STEPS; i < ROWS_HALO_STEPS + ROWS_RESULT_STEPS + ROWS_HALO_STEPS; i++)
                    l_Data[lidY][lidX + i * ROWS_BLOCKDIM_X] = (baseX + i * ROWS_BLOCKDIM_X < imageW) ? src[i * ROWS_BLOCKDIM_X] : 0;

                    //Compute and store results

                    #pragma unroll
                    for(int i = ROWS_HALO_STEPS; i < ROWS_HALO_STEPS + ROWS_RESULT_STEPS; i++) {
                    float sum = 0;

                    #pragma unroll
                    for(int j = -KERNEL_RADIUS; j <= KERNEL_RADIUS; j++)
                    sum += kernel[KERNEL_RADIUS - j] * l_Data[lidY][lidX + i * ROWS_BLOCKDIM_X + j];

                    dst[i * ROWS_BLOCKDIM_X] = sum;
                    }

                }
            }
        }
    }
}
extern "C"

void conv_cols(
    float * dst,
    const float * src,
    const float * kernel,
    const int imageW,
    const int imageH,
    const int pitch)
{
    #pragma HLS INTERFACE m_axi port=dst offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=kernel offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=imageW
    #pragma HLS INTERFACE s_axilite port=imageH
    #pragma HLS INTERFACE s_axilite port=pitch
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=l_Data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=l_Data complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float l_Data[COLUMNS_BLOCKDIM_X][(COLUMNS_RESULT_STEPS + 2 * COLUMNS_HALO_STEPS) * COLUMNS_BLOCKDIM_Y + 1];

                    int gidX = _bid_x;
                    int gidY = _bid_y;
                    int lidX = _tid_x;
                    int lidY = _tid_y;

                    //Offset to the upper halo edge
                    const int baseX = gidX * COLUMNS_BLOCKDIM_X + lidX;
                    const int baseY = (gidY * COLUMNS_RESULT_STEPS - COLUMNS_HALO_STEPS) * COLUMNS_BLOCKDIM_Y + lidY;
                    src += baseY * pitch + baseX;
                    dst += baseY * pitch + baseX;

                    //Load main data
                    #pragma unroll
                    for(int i = COLUMNS_HALO_STEPS; i < COLUMNS_HALO_STEPS + COLUMNS_RESULT_STEPS; i++)
                    l_Data[lidX][lidY + i * COLUMNS_BLOCKDIM_Y] = src[i * COLUMNS_BLOCKDIM_Y * pitch];

                    //Load upper halo
                    #pragma unroll
                    for(int i = 0; i < COLUMNS_HALO_STEPS; i++)
                    l_Data[lidX][lidY + i * COLUMNS_BLOCKDIM_Y] = (baseY + i * COLUMNS_BLOCKDIM_Y >= 0) ? src[i * COLUMNS_BLOCKDIM_Y * pitch] : 0;

                    //Load lower halo
                    #pragma unroll
                    for(int i = COLUMNS_HALO_STEPS + COLUMNS_RESULT_STEPS; i < COLUMNS_HALO_STEPS + COLUMNS_RESULT_STEPS + COLUMNS_HALO_STEPS; i++)
                    l_Data[lidX][lidY + i * COLUMNS_BLOCKDIM_Y] = (baseY + i * COLUMNS_BLOCKDIM_Y < imageH) ? src[i * COLUMNS_BLOCKDIM_Y * pitch] : 0;

                    //Compute and store results

                    #pragma unroll
                    for(int i = COLUMNS_HALO_STEPS; i < COLUMNS_HALO_STEPS + COLUMNS_RESULT_STEPS; i++) {
                    float sum = 0;

                    #pragma unroll
                    for(int j = -KERNEL_RADIUS; j <= KERNEL_RADIUS; j++)
                    sum += kernel[KERNEL_RADIUS - j] * l_Data[lidX][lidY + i * COLUMNS_BLOCKDIM_Y + j];

                    dst[i * COLUMNS_BLOCKDIM_Y * pitch] = sum;
                    }

                }
            }
        }
    }
}
