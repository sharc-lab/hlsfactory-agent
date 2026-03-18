#include "kernel.h"

// --- from main.cu ---
extern "C"
void winograd_conv2d(
    const DATA_TYPE * input,
    const DATA_TYPE * transformed_filter ,
    DATA_TYPE * output,
    const int offset_i,
    const int offset_j)
{
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=transformed_filter offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=offset_i
    #pragma HLS INTERFACE s_axilite port=offset_j
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int tile_i = _bid_x * BLOCK_DIM_X + _tid_x + offset_i;
                    int tile_j = _bid_y * BLOCK_DIM_Y + _tid_y + offset_j;

                    // input transformation

                    DATA_TYPE input_tile[4][4], tmp_tile[4][4], transformed_tile[4][4];
                    for (int i = 0; i < 4; i ++) {
                    for (int j = 0; j < 4; j ++) {
                    int x = 2 * tile_i + i;
                    int y = 2 * tile_j + j;
                    if (x >= MAP_SIZE || y >= MAP_SIZE) {
                    input_tile[i][j] = 0;
                    continue;
                    }
                    input_tile[i][j] = input[x * MAP_SIZE + y];
                    }
                    }

                    // Bt * d
                    for (int j = 0; j < 4; j ++) {
                    tmp_tile[0][j] = input_tile[0][j] - input_tile[2][j];
                    tmp_tile[1][j] = input_tile[1][j] + input_tile[2][j];
                    tmp_tile[2][j] = -input_tile[1][j] + input_tile[2][j];
                    tmp_tile[3][j] = input_tile[1][j] - input_tile[3][j];
                    }
                    // d * B
                    for (int i = 0; i < 4; i ++) {
                    transformed_tile[i][0] = tmp_tile[i][0] - tmp_tile[i][2];
                    transformed_tile[i][1] = tmp_tile[i][1] + tmp_tile[i][2];
                    transformed_tile[i][2] = -tmp_tile[i][1] + tmp_tile[i][2];
                    transformed_tile[i][3] = tmp_tile[i][1] - tmp_tile[i][3];
                    }

                    // element-wise multiplication

                    DATA_TYPE multiplied_tile[4][4];
                    for (int i = 0; i < 4; i ++) {
                    for (int j = 0; j < 4; j ++) {
                    multiplied_tile[i][j] = transformed_tile[i][j] * transformed_filter[i * 4 + j];
                    }
                    }

                    // output transformation

                    DATA_TYPE tmp_tile_1[2][4], final_tile[2][2];

                    // At * I
                    for (int j = 0; j < 4; j ++) {
                    tmp_tile_1[0][j] = multiplied_tile[0][j] + multiplied_tile[1][j] + multiplied_tile[2][j];
                    tmp_tile_1[1][j] = multiplied_tile[1][j] - multiplied_tile[2][j] - multiplied_tile[3][j];
                    }
                    // I * A
                    for (int i = 0; i < 2; i ++) {
                    final_tile[i][0] = tmp_tile_1[i][0] + tmp_tile_1[i][1] + tmp_tile_1[i][2];
                    final_tile[i][1] = tmp_tile_1[i][1] - tmp_tile_1[i][2] - tmp_tile_1[i][3];
                    }

                    for (int i = 0; i < 2; i ++) {
                    for (int j = 0; j < 2; j ++) {
                    int x = 2 * tile_i + i;
                    int y = 2 * tile_j + j;
                    if (x >= MAP_SIZE - 2 || y >= MAP_SIZE - 2) {
                    continue;
                    }
                    output[x * (MAP_SIZE - 2) + y] = final_tile[i][j];
                    }
                    }

                }
            }
        }
    }
}
