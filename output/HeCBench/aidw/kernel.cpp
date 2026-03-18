#include "kernel.h"

// --- from main.cu ---
extern "C"
void AIDW_Kernel(
    const float *__restrict dx, 
    const float *__restrict dy,
    const float *__restrict dz,
    const int dnum,
    const float *__restrict ix,
    const float *__restrict iy,
          float *__restrict iz,
    const int inum,
    const float area,
    const float *__restrict avg_dist) 

{
    #pragma HLS INTERFACE m_axi port=dx offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dy offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dz offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=dnum
    #pragma HLS INTERFACE m_axi port=ix offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=iy offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=iz offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=inum
    #pragma HLS INTERFACE s_axilite port=area
    #pragma HLS INTERFACE m_axi port=avg_dist offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int tid = _bid_x * BLOCK_DIM_X + _tid_x;
            if (tid >= inum) return;

            float sum = 0.f, dist = 0.f, t = 0.f, z = 0.f, alpha = 1.f;

            float r_obs = avg_dist[tid];                // The observed average nearest neighbor distance
            float r_exp = 0.5f / sqrtf(dnum / area);    // The expected nearest neighbor distance for a random pattern
            float R_S0 = r_obs / r_exp;                 // The nearest neighbor statistic

            // Normalize the R(S0) measure such that it is bounded by 0 and 1 by a fuzzy membership function
            float u_R = 0.f;
            if(R_S0 >= R_min) u_R = 0.5f-0.5f * cosf(3.1415926f / R_max * (R_S0 - R_min));
            if(R_S0 >= R_max) u_R = 1.f;

            // Determine the appropriate distance-decay parameter alpha by a triangular membership function
            // Adaptive power parameter: a (alpha)
            if(u_R>= 0.f && u_R<=0.1f)  alpha = a1;
            if(u_R>0.1f && u_R<=0.3f)  alpha = a1*(1.f-5.f*(u_R-0.1f)) + a2*5.f*(u_R-0.1f);
            if(u_R>0.3f && u_R<=0.5f)  alpha = a3*5.f*(u_R-0.3f) + a1*(1.f-5.f*(u_R-0.3f));
            if(u_R>0.5f && u_R<=0.7f)  alpha = a3*(1.f-5.f*(u_R-0.5f)) + a4*5.f*(u_R-0.5f);
            if(u_R>0.7f && u_R<=0.9f)  alpha = a5*5.f*(u_R-0.7f) + a4*(1.f-5.f*(u_R-0.7f));
            if(u_R>0.9f && u_R<=1.f)  alpha = a5;
            alpha *= 0.5f; // Half of the power

            // Weighted average
            for(int j = 0; j < dnum; j++) {
            dist = (ix[tid] - dx[j]) * (ix[tid] - dx[j]) + (iy[tid] - dy[j]) * (iy[tid] - dy[j]) ;
            t = 1.f / powf(dist, alpha);  sum += t;  z += dz[j] * t;
            }
            iz[tid] = z / sum;

        }
    }
}
extern "C"

void AIDW_Kernel_Tiled(
    const float *__restrict dx, 
    const float *__restrict dy,
    const float *__restrict dz,
    const int dnum,
    const float *__restrict ix,
    const float *__restrict iy,
          float *__restrict iz,
    const int inum,
    const float area,
    const float *__restrict avg_dist)
{
    #pragma HLS INTERFACE m_axi port=dx offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dy offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dz offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=dnum
    #pragma HLS INTERFACE m_axi port=ix offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=iy offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=iz offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=inum
    #pragma HLS INTERFACE s_axilite port=area
    #pragma HLS INTERFACE m_axi port=avg_dist offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sdx complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sdy complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sdz complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            // Shared Memory
            float sdx[BLOCK_SIZE];
            float sdy[BLOCK_SIZE];
            float sdz[BLOCK_SIZE];

            int tid = _tid_x + _bid_x * BLOCK_DIM_X;
            if (tid >= inum) return;

            float dist = 0.f, t = 0.f, alpha = 0.f;

            int part = (dnum - 1) / BLOCK_SIZE;
            int m, e;

            float sum_up = 0.f;
            float sum_dn = 0.f;
            float six_s, siy_s;

            float r_obs = avg_dist[tid];               //The observed average nearest neighbor distance
            float r_exp = 0.5f / sqrtf(dnum / area); // The expected nearest neighbor distance for a random pattern
            float R_S0 = r_obs / r_exp;                //The nearest neighbor statistic

            float u_R = 0.f;
            if(R_S0 >= R_min) u_R = 0.5f-0.5f * cosf(3.1415926f / R_max * (R_S0 - R_min));
            if(R_S0 >= R_max) u_R = 1.f;

            // Determine the appropriate distance-decay parameter alpha by a triangular membership function
            // Adaptive power parameter: a (alpha)
            if(u_R>= 0.f && u_R<=0.1f)  alpha = a1;
            if(u_R>0.1f && u_R<=0.3f)  alpha = a1*(1.f-5.f*(u_R-0.1f)) + a2*5.f*(u_R-0.1f);
            if(u_R>0.3f && u_R<=0.5f)  alpha = a3*5.f*(u_R-0.3f) + a1*(1.f-5.f*(u_R-0.3f));
            if(u_R>0.5f && u_R<=0.7f)  alpha = a3*(1.f-5.f*(u_R-0.5f)) + a4*5.f*(u_R-0.5f);
            if(u_R>0.7f && u_R<=0.9f)  alpha = a5*5.f*(u_R-0.7f) + a4*(1.f-5.f*(u_R-0.7f));
            if(u_R>0.9f && u_R<=1.f)  alpha = a5;
            alpha *= 0.5f; // Half of the power

            float six_t = ix[tid];
            float siy_t = iy[tid];
            int lid = _tid_x;
            for(m = 0; m <= part; m++) {  // Weighted Sum
            int num_threads = min(BLOCK_SIZE, dnum - BLOCK_SIZE *m);
            if (lid < num_threads) {
            sdx[lid] = dx[lid + BLOCK_SIZE * m];
            sdy[lid] = dy[lid + BLOCK_SIZE * m];
            sdz[lid] = dz[lid + BLOCK_SIZE * m];
            }

            for(e = 0; e < BLOCK_SIZE; e++) {
            six_s = six_t - sdx[e];
            siy_s = siy_t - sdy[e];
            dist = (six_s * six_s + siy_s * siy_s);
            t = 1.f / powf(dist, alpha);  sum_dn += t;  sum_up += t * sdz[e];
            }
            }
            iz[tid] = sum_up / sum_dn;

        }
    }
}
