#include "kernel.h"

// --- from main.cu ---
float gammafunction(unsigned int n)
{   
  if(n == 0)
    return 0.0f;
  float x = ((float)n + 0.5f) * logf((float) n) - ((float)n - 1.0f);
  return x;
}
extern "C"

void epi(const unsigned int* dev_data_zeros, 
                    const unsigned int* dev_data_ones, 
                    float* dev_scores, 
                    const int num_snp, 
                    const int PP_zeros, 
                    const int PP_ones,
                    const int mask_zeros, 
                    const int mask_ones) 
{
    #pragma HLS INTERFACE m_axi port=dev_data_zeros offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dev_data_ones offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dev_scores offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=num_snp
    #pragma HLS INTERFACE s_axilite port=PP_zeros
    #pragma HLS INTERFACE s_axilite port=PP_ones
    #pragma HLS INTERFACE s_axilite port=mask_zeros
    #pragma HLS INTERFACE s_axilite port=mask_ones
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int i, j, tid, p, k;

                    j = BLOCK_DIM_X * _bid_x + _tid_x;
                    i = BLOCK_DIM_Y * _bid_y + _tid_y;
                    tid = i * num_snp + j;

                    if (j > i && i < num_snp && j < num_snp) {
                    unsigned int ft[2 * 9];
                    for(k = 0; k < 2 * 9; k++) ft[k] = 0;

                    unsigned int t00, t01, t02, t10, t11, t12, t20, t21, t22;
                    unsigned int di2, dj2;
                    unsigned int* SNPi;
                    unsigned int* SNPj;

                    // Phenotype 0
                    SNPi = (unsigned int*) &dev_data_zeros[i * 2];
                    SNPj = (unsigned int*) &dev_data_zeros[j * 2];

                    #pragma unroll 1
                    for (p = 0; p < 2 * PP_zeros * num_snp - 2 * num_snp; p += 2 * num_snp) {
                    di2 = ~(SNPi[p] | SNPi[p + 1]);
                    dj2 = ~(SNPj[p] | SNPj[p + 1]);

                    t00 = SNPi[p] & SNPj[p];
                    t01 = SNPi[p] & SNPj[p + 1];
                    t02 = SNPi[p] & dj2;
                    t10 = SNPi[p + 1] & SNPj[p];
                    t11 = SNPi[p + 1] & SNPj[p + 1];
                    t12 = SNPi[p + 1] & dj2;
                    t20 = di2 & SNPj[p];
                    t21 = di2 & SNPj[p + 1];
                    t22 = di2 & dj2;

                    ft[0] += __builtin_popcount(t00);
                    ft[1] += __builtin_popcount(t01);
                    ft[2] += __builtin_popcount(t02);
                    ft[3] += __builtin_popcount(t10);
                    ft[4] += __builtin_popcount(t11);
                    ft[5] += __builtin_popcount(t12);
                    ft[6] += __builtin_popcount(t20);
                    ft[7] += __builtin_popcount(t21);
                    ft[8] += __builtin_popcount(t22);
                    }

                    // remainder
                    p = 2 * PP_zeros * num_snp - 2 * num_snp;
                    di2 = ~(SNPi[p] | SNPi[p + 1]);
                    dj2 = ~(SNPj[p] | SNPj[p + 1]);
                    di2 = di2 & mask_zeros;
                    dj2 = dj2 & mask_zeros;

                    t00 = SNPi[p] & SNPj[p];
                    t01 = SNPi[p] & SNPj[p + 1];
                    t02 = SNPi[p] & dj2;
                    t10 = SNPi[p + 1] & SNPj[p];
                    t11 = SNPi[p + 1] & SNPj[p + 1];
                    t12 = SNPi[p + 1] & dj2;
                    t20 = di2 & SNPj[p];
                    t21 = di2 & SNPj[p + 1];
                    t22 = di2 & dj2;

                    ft[0] += __builtin_popcount(t00);
                    ft[1] += __builtin_popcount(t01);
                    ft[2] += __builtin_popcount(t02);
                    ft[3] += __builtin_popcount(t10);
                    ft[4] += __builtin_popcount(t11);
                    ft[5] += __builtin_popcount(t12);
                    ft[6] += __builtin_popcount(t20);
                    ft[7] += __builtin_popcount(t21);
                    ft[8] += __builtin_popcount(t22);

                    // Phenotype 1
                    SNPi = (unsigned int*) &dev_data_ones[i * 2];
                    SNPj = (unsigned int*) &dev_data_ones[j * 2];

                    #pragma unroll 1
                    for(p = 0; p < 2 * PP_ones * num_snp - 2 * num_snp; p += 2 * num_snp)
                    {
                    di2 = ~(SNPi[p] | SNPi[p + 1]);
                    dj2 = ~(SNPj[p] | SNPj[p + 1]);

                    t00 = SNPi[p] & SNPj[p];
                    t01 = SNPi[p] & SNPj[p + 1];
                    t02 = SNPi[p] & dj2;
                    t10 = SNPi[p + 1] & SNPj[p];
                    t11 = SNPi[p + 1] & SNPj[p + 1];
                    t12 = SNPi[p + 1] & dj2;
                    t20 = di2 & SNPj[p];
                    t21 = di2 & SNPj[p + 1];
                    t22 = di2 & dj2;

                    ft[9]  += __builtin_popcount(t00);
                    ft[10] += __builtin_popcount(t01);
                    ft[11] += __builtin_popcount(t02);
                    ft[12] += __builtin_popcount(t10);
                    ft[13] += __builtin_popcount(t11);
                    ft[14] += __builtin_popcount(t12);
                    ft[15] += __builtin_popcount(t20);
                    ft[16] += __builtin_popcount(t21);
                    ft[17] += __builtin_popcount(t22);
                    }
                    p = 2 * PP_ones * num_snp - 2 * num_snp;
                    di2 = ~(SNPi[p] | SNPi[p + 1]);
                    dj2 = ~(SNPj[p] | SNPj[p + 1]);
                    di2 = di2 & mask_ones;
                    dj2 = dj2 & mask_ones;

                    t00 = SNPi[p] & SNPj[p];
                    t01 = SNPi[p] & SNPj[p + 1];
                    t02 = SNPi[p] & dj2;
                    t10 = SNPi[p + 1] & SNPj[p];
                    t11 = SNPi[p + 1] & SNPj[p + 1];
                    t12 = SNPi[p + 1] & dj2;
                    t20 = di2 & SNPj[p];
                    t21 = di2 & SNPj[p + 1];
                    t22 = di2 & dj2;

                    ft[9]  += __builtin_popcount(t00);
                    ft[10] += __builtin_popcount(t01);
                    ft[11] += __builtin_popcount(t02);
                    ft[12] += __builtin_popcount(t10);
                    ft[13] += __builtin_popcount(t11);
                    ft[14] += __builtin_popcount(t12);
                    ft[15] += __builtin_popcount(t20);
                    ft[16] += __builtin_popcount(t21);
                    ft[17] += __builtin_popcount(t22);

                    // compute score
                    float score = 0.0f;

                    #pragma unroll
                    for(k = 0; k < 9; k++)
                    score += gammafunction(ft[k] + ft[9 + k] + 1) -
                    gammafunction(ft[k]) - gammafunction(ft[9 + k]);
                    score = fabsf(score);
                    if(score == 0.0f)
                    score = FLT_MAX;
                    dev_scores[tid] = score;
                    }

                }
            }
        }
    }
}
