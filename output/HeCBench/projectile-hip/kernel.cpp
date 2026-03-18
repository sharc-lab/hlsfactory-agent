#include "kernel.h"

// --- from Projectile.cu ---
extern "C"
void CalculateRange(const Projectile *obj, Projectile *pObj) {
    #pragma HLS INTERFACE m_axi port=obj offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=pObj offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            int i = BLOCK_DIM_X*_bid_x + _tid_x;
            if (i >= num_elements) return;
            float proj_angle = obj[i].getangle();
            float proj_vel = obj[i].getvelocity();
            float sin_value = sinf(proj_angle * kPIValue / 180.0f);
            float cos_value = cosf(proj_angle * kPIValue / 180.0f);
            float total_time = fabsf((2 * proj_vel * sin_value)) / kGValue;
            float max_range =  fabsf(proj_vel * total_time * cos_value);
            float max_height = (proj_vel * proj_vel * sin_value * sin_value) / 2.0f *
            kGValue;  // h = v^2 * sin^2theta/2g

            pObj[i].setRangeandTime(max_range, total_time, proj_angle, proj_vel, max_height);

        }
    }
}
