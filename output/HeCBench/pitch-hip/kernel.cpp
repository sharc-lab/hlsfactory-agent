#include "kernel.h"

// --- from main.cu ---
float sigmoid (float x) {
  return (1.f / (1.f + expf(-x)));
}
extern "C"

void parallelPitched2DAccess (float* devPtr, size_t pitch, int width, int height)
{
    #pragma HLS INTERFACE m_axi port=devPtr offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=pitch
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            int r = _bid_y * BLOCK_DIM_Y + _tid_y;
                            int c = _bid_x * BLOCK_DIM_X + _tid_x;
                            if (r < height && c < width) {
                            float* row = (float*)((char*)devPtr + r * pitch);
                            row[c] = sigmoid(row[c]);
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void parallelSimple2DAccess (float* elem, int width, int height)
{
    #pragma HLS INTERFACE m_axi port=elem offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            int r = _bid_y * BLOCK_DIM_Y + _tid_y;
                            int c = _bid_x * BLOCK_DIM_X + _tid_x;
                            if (r < height && c < width) {
                            elem[r * width + c] = sigmoid(elem[r * width + c]);
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void parallelPitched3DAccess (hipPitchedPtr devPitchedPtr, int width, int height, int depth)
{
    #pragma HLS INTERFACE s_axilite port=devPitchedPtr
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=depth
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            int z = _bid_z * BLOCK_DIM_Z + _tid_z;
                            int y = _bid_y * BLOCK_DIM_Y + _tid_y;
                            int x = _bid_x * BLOCK_DIM_X + _tid_x;
                            if (z < depth && y < height && x < width) {
                            char* devPtr = (char*)devPitchedPtr.ptr;
                            size_t pitch = devPitchedPtr.pitch;
                            size_t slicePitch = pitch * height;
                            char* slice = devPtr + z * slicePitch;
                            float* row = (float*)(slice + y * pitch);
                            row[x] = sigmoid(row[x]);
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void parallelSimple3DAccess (float* elem, int width, int height, int depth)
{
    #pragma HLS INTERFACE m_axi port=elem offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=depth
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            int z = _bid_z * BLOCK_DIM_Z + _tid_z;
                            int y = _bid_y * BLOCK_DIM_Y + _tid_y;
                            int x = _bid_x * BLOCK_DIM_X + _tid_x;
                            if (z < depth && y < height && x < width) {
                            float element = elem[z * height * width + y * width + x];
                            elem[z * height * width + y * width + x] = sigmoid(element);
                            }

                        }
                    }
                }
            }
        }
    }
}
