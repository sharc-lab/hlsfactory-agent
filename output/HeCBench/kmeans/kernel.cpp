#include "kernel.h"

// --- from cluster.cu ---
extern "C"
void find_membership (const float* feature,
                      const float* cluster,
                              int* member, 
                      const int nclusters,
                      const int nfeatures,
                      const int npoints)
{
    #pragma HLS INTERFACE m_axi port=feature offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=cluster offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=member offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=nclusters
    #pragma HLS INTERFACE s_axilite port=nfeatures
    #pragma HLS INTERFACE s_axilite port=npoints
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int point_id = _bid_x * BLOCK_DIM_X + _tid_x;
            if (point_id < npoints) {
            int index = 0;
            float min_dist = FLT_MAX;
            for (int i = 0; i < nclusters; i++) {
            float dist = 0;
            float ans  = 0;
            for (int l = 0; l < nfeatures; l++) {
            ans += (feature[l * npoints + point_id] - cluster[i * nfeatures + l]) *
            (feature[l * npoints + point_id] - cluster[i * nfeatures + l]) ;
            }
            dist = ans;
            if (dist < min_dist) {
            min_dist = dist;
            index    = i;
            }
            }
            member[point_id] = index;
            }

        }
    }
}
