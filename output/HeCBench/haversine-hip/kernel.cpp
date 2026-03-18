#include "kernel.h"

// --- from distance.cu ---
extern "C"
void compute_haversine_distance(
  const double4 * p,
        double* distance,
  const int n)
{
    #pragma HLS INTERFACE m_axi port=p offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=distance offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i < n) {
            auto ay = p[i].x * DEGREE_TO_RADIAN;  // a_lat
            auto ax = p[i].y * DEGREE_TO_RADIAN;  // a_lon
            auto by = p[i].z * DEGREE_TO_RADIAN;  // b_lat
            auto bx = p[i].w * DEGREE_TO_RADIAN;  // b_lon

            // haversine formula
            auto x        = (bx - ax) / 2.0;
            auto y        = (by - ay) / 2.0;
            auto sinysqrd = sin(y) * sin(y);
            auto sinxsqrd = sin(x) * sin(x);
            auto scale    = cos(ay) * cos(by);
            distance[i] = 2.0 * EARTH_RADIUS_KM * asin(sqrt(sinysqrd + sinxsqrd * scale));
            }

        }
    }
}
