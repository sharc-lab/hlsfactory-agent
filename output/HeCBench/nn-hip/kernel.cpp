#include "kernel.h"

// --- from nearestNeighbor.cu ---
extern "C"
void nn (const int numRecords, const float lat, const float lng,
    const LatLong * locations,
    float* distances) 
{
    #pragma HLS INTERFACE s_axilite port=numRecords
    #pragma HLS INTERFACE s_axilite port=lat
    #pragma HLS INTERFACE s_axilite port=lng
    #pragma HLS INTERFACE m_axi port=locations offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=distances offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int gid = BLOCK_DIM_X * _bid_x + _tid_x;
            if (gid < numRecords) {
            LatLong latLong = locations[gid];
            distances[gid] = sqrtf((lat-latLong.lat)*(lat-latLong.lat)+
            (lng-latLong.lng)*(lng-latLong.lng));
            }

        }
    }
}
