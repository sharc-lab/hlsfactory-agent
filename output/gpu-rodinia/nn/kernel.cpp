#include "kernel.h"

// --- from nn_cuda.cu ---
extern "C"
void euclid(LatLong *d_locations, float *d_distances, int numRecords,float lat, float lng)
{
    #pragma HLS INTERFACE m_axi port=d_locations offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_distances offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=numRecords
    #pragma HLS INTERFACE s_axilite port=lat
    #pragma HLS INTERFACE s_axilite port=lng
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                //int globalId = GRID_DIM_X * BLOCK_DIM_X * _bid_y + BLOCK_DIM_X * _bid_x + _tid_x;
                int globalId = BLOCK_DIM_X * ( GRID_DIM_X * _bid_y + _bid_x ) + _tid_x; // more efficient
                LatLong *latLong = d_locations+globalId;
                if (globalId < numRecords) {
                float *dist=d_distances+globalId;
                *dist = (float)sqrt((lat-latLong->lat)*(lat-latLong->lat)+(lng-latLong->lng)*(lng-latLong->lng));
                }

            }
        }
    }
}
