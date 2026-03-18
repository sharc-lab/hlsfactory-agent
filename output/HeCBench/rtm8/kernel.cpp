#include "kernel.h"

// --- from rtm8.cu ---
inline int indexTo1D(int x, int y, int z){
  return x + y*nx + z*nx*ny;
}
extern "C"

void rtm8(
  const float* vsq,
  const float* current_s,
  const float* current_r,
        float* next_s,
        float* next_r,
        float* image,
  const float* a,
  size_t N)
{
    #pragma HLS INTERFACE m_axi port=vsq offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=current_s offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=current_r offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=next_s offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=next_r offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=image offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            unsigned x = _bid_x * BLOCK_DIM_X + _tid_x;
                            unsigned y = _bid_y * BLOCK_DIM_Y + _tid_y;
                            unsigned z = _bid_z * BLOCK_DIM_Z + _tid_z;
                            float div;
                            if ((4 <= x && x < (nx - 4) ) && (4 <= y && y < (ny - 4)) && (4 <= z && z < (nz - 4))){
                            div =
                            a[0] * current_s[indexTo1D(x,y,z)] +
                            a[1] * (current_s[indexTo1D(x+1,y,z)] + current_s[indexTo1D(x-1,y,z)] +
                            current_s[indexTo1D(x,y+1,z)] + current_s[indexTo1D(x,y-1,z)] +
                            current_s[indexTo1D(x,y,z+1)] + current_s[indexTo1D(x,y,z-1)]) +
                            a[2] * (current_s[indexTo1D(x+2,y,z)] + current_s[indexTo1D(x-2,y,z)] +
                            current_s[indexTo1D(x,y+2,z)] + current_s[indexTo1D(x,y-2,z)] +
                            current_s[indexTo1D(x,y,z+2)] + current_s[indexTo1D(x,y,z-2)]) +
                            a[3] * (current_s[indexTo1D(x+3,y,z)] + current_s[indexTo1D(x-3,y,z)] +
                            current_s[indexTo1D(x,y+3,z)] + current_s[indexTo1D(x,y-3,z)] +
                            current_s[indexTo1D(x,y,z+3)] + current_s[indexTo1D(x,y,z-3)]) +
                            a[4] * (current_s[indexTo1D(x+4,y,z)] + current_s[indexTo1D(x-4,y,z)] +
                            current_s[indexTo1D(x,y+4,z)] + current_s[indexTo1D(x,y-4,z)] +
                            current_s[indexTo1D(x,y,z+4)] + current_s[indexTo1D(x,y,z-4)]);

                            next_s[indexTo1D(x,y,z)] = 2*current_s[indexTo1D(x,y,z)] - next_s[indexTo1D(x,y,z)]
                            + vsq[indexTo1D(x,y,z)]*div;
                            div =
                            a[0] * current_r[indexTo1D(x,y,z)] +
                            a[1] * (current_r[indexTo1D(x+1,y,z)] + current_r[indexTo1D(x-1,y,z)] +
                            current_r[indexTo1D(x,y+1,z)] + current_r[indexTo1D(x,y-1,z)] +
                            current_r[indexTo1D(x,y,z+1)] + current_r[indexTo1D(x,y,z-1)]) +
                            a[2] * (current_r[indexTo1D(x+2,y,z)] + current_r[indexTo1D(x-2,y,z)] +
                            current_r[indexTo1D(x,y+2,z)] + current_r[indexTo1D(x,y-2,z)] +
                            current_r[indexTo1D(x,y,z+2)] + current_r[indexTo1D(x,y,z-2)]) +
                            a[3] * (current_r[indexTo1D(x+3,y,z)] + current_r[indexTo1D(x-3,y,z)] +
                            current_r[indexTo1D(x,y+3,z)] + current_r[indexTo1D(x,y-3,z)] +
                            current_r[indexTo1D(x,y,z+3)] + current_r[indexTo1D(x,y,z-3)]) +
                            a[4] * (current_r[indexTo1D(x+4,y,z)] + current_r[indexTo1D(x-4,y,z)] +
                            current_r[indexTo1D(x,y+4,z)] + current_r[indexTo1D(x,y-4,z)] +
                            current_r[indexTo1D(x,y,z+4)] + current_r[indexTo1D(x,y,z-4)]);

                            next_r[indexTo1D(x,y,z)] = 2 * current_r[indexTo1D(x,y,z)]
                            - next_r[indexTo1D(x,y,z)] + vsq[indexTo1D(x,y,z)] * div;

                            image[indexTo1D(x,y,z)] = next_s[indexTo1D(x,y,z)] * next_r[indexTo1D(x,y,z)];
                            }

                        }
                    }
                }
            }
        }
    }
}
