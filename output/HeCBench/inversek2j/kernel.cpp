#include "kernel.h"

// --- from main.cu ---
extern "C"
void invkin_kernel(
  const float * xTarget_in,
  const float * yTarget_in,
        float * angles,
  int size)
{
    #pragma HLS INTERFACE m_axi port=xTarget_in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=yTarget_in offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=angles offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int blockId = _bid_x + _bid_y * GRID_DIM_X;
                    int idx = blockId * (BLOCK_DIM_X * BLOCK_DIM_Y) + (_tid_y * BLOCK_DIM_X) + _tid_x;

                    if(idx < size)
                    {
                    float angle_out[NUM_JOINTS];
                    float curr_xTargetIn = xTarget_in[idx];
                    float curr_yTargetIn = yTarget_in[idx];

                    for(int i = 0; i < NUM_JOINTS; i++)
                    {
                    angle_out[i] = 0.0;
                    }

                    float angle;
                    // Initialize x and y data
                    float xData[NUM_JOINTS_P1];
                    float yData[NUM_JOINTS_P1];

                    for (int i = 0 ; i < NUM_JOINTS_P1; i++)
                    {
                    xData[i] = i;
                    yData[i] = 0.f;
                    }

                    for(int curr_loop = 0; curr_loop < MAX_LOOP; curr_loop++)
                    {
                    for (int iter = NUM_JOINTS; iter > 0; iter--)
                    {
                    float pe_x = xData[NUM_JOINTS];
                    float pe_y = yData[NUM_JOINTS];
                    float pc_x = xData[iter-1];
                    float pc_y = yData[iter-1];
                    float diff_pe_pc_x = pe_x - pc_x;
                    float diff_pe_pc_y = pe_y - pc_y;
                    float diff_tgt_pc_x = curr_xTargetIn - pc_x;
                    float diff_tgt_pc_y = curr_yTargetIn - pc_y;
                    float len_diff_pe_pc = sqrtf(diff_pe_pc_x * diff_pe_pc_x + diff_pe_pc_y * diff_pe_pc_y);
                    float len_diff_tgt_pc = sqrtf(diff_tgt_pc_x * diff_tgt_pc_x + diff_tgt_pc_y * diff_tgt_pc_y);
                    float a_x = diff_pe_pc_x / len_diff_pe_pc;
                    float a_y = diff_pe_pc_y / len_diff_pe_pc;
                    float b_x = diff_tgt_pc_x / len_diff_tgt_pc;
                    float b_y = diff_tgt_pc_y / len_diff_tgt_pc;
                    float a_dot_b = a_x * b_x + a_y * b_y;
                    if (a_dot_b > 1.f)
                    a_dot_b = 1.f;
                    else if (a_dot_b < -1.f)
                    a_dot_b = -1.f;
                    angle = acosf(a_dot_b) * (180.f / PI);
                    // Determine angle direction
                    float direction = a_x * b_y - a_y * b_x;
                    if (direction < 0.f)
                    angle = -angle;
                    // Make the result look more natural (these checks may be omitted)
                    if (angle > 30.f)
                    angle = 30.f;
                    else if (angle < -30.f)
                    angle = -30.f;
                    // Save angle
                    angle_out[iter - 1] = angle;
                    for (int i = 0; i < NUM_JOINTS; i++)
                    {
                    if(i < NUM_JOINTS - 1)
                    {
                    angle_out[i+1] += angle_out[i];
                    }
                    }
                    }
                    }

                    angles[idx * NUM_JOINTS + 0] = angle_out[0];
                    angles[idx * NUM_JOINTS + 1] = angle_out[1];
                    angles[idx * NUM_JOINTS + 2] = angle_out[2];
                    }

                }
            }
        }
    }
}
