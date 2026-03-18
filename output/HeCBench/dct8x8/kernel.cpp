#include "kernel.h"

// --- from DCT8x8_gold.cu ---
static void IDCT8(float *dst, const float *src, unsigned int ostride, unsigned int istride){
    float Y04P   = src[0 * istride] + src[4 * istride];
    float Y2b6eP = C_b * src[2 * istride] + C_e * src[6 * istride];

    float Y04P2b6ePP = Y04P + Y2b6eP;
    float Y04P2b6ePM = Y04P - Y2b6eP;
    float Y7f1aP3c5dPP = C_f * src[7 * istride] + C_a * src[1 * istride] + C_c * src[3 * istride] + C_d * src[5 * istride];
    float Y7a1fM3d5cMP = C_a * src[7 * istride] - C_f * src[1 * istride] + C_d * src[3 * istride] - C_c * src[5 * istride];

    float Y04M   = src[0*istride] - src[4*istride];
    float Y2e6bM = C_e * src[2*istride] - C_b * src[6*istride];

    float Y04M2e6bMP = Y04M + Y2e6bM;
    float Y04M2e6bMM = Y04M - Y2e6bM;
    float Y1c7dM3f5aPM = C_c * src[1 * istride] - C_d * src[7 * istride] - C_f * src[3 * istride] - C_a * src[5 * istride];
    float Y1d7cP3a5fMM = C_d * src[1 * istride] + C_c * src[7 * istride] - C_a * src[3 * istride] + C_f * src[5 * istride];

    dst[0 * ostride] = C_norm * (Y04P2b6ePP + Y7f1aP3c5dPP);
    dst[7 * ostride] = C_norm * (Y04P2b6ePP - Y7f1aP3c5dPP);
    dst[4 * ostride] = C_norm * (Y04P2b6ePM + Y7a1fM3d5cMP);
    dst[3 * ostride] = C_norm * (Y04P2b6ePM - Y7a1fM3d5cMP);

    dst[1 * ostride] = C_norm * (Y04M2e6bMP + Y1c7dM3f5aPM);
    dst[5 * ostride] = C_norm * (Y04M2e6bMM - Y1d7cP3a5fMM);
    dst[2 * ostride] = C_norm * (Y04M2e6bMM + Y1d7cP3a5fMM);
    dst[6 * ostride] = C_norm * (Y04M2e6bMP - Y1c7dM3f5aPM);
}


// --- from kernels.cu ---
inline void IDCT8(float *D){
    float Y04P   = D[0] + D[4];
    float Y2b6eP = C_b * D[2] + C_e * D[6];

    float Y04P2b6ePP = Y04P + Y2b6eP;
    float Y04P2b6ePM = Y04P - Y2b6eP;
    float Y7f1aP3c5dPP = C_f * D[7] + C_a * D[1] + C_c * D[3] + C_d * D[5];
    float Y7a1fM3d5cMP = C_a * D[7] - C_f * D[1] + C_d * D[3] - C_c * D[5];

    float Y04M   = D[0] - D[4];
    float Y2e6bM = C_e * D[2] - C_b * D[6];

    float Y04M2e6bMP = Y04M + Y2e6bM;
    float Y04M2e6bMM = Y04M - Y2e6bM;
    float Y1c7dM3f5aPM = C_c * D[1] - C_d * D[7] - C_f * D[3] - C_a * D[5];
    float Y1d7cP3a5fMM = C_d * D[1] + C_c * D[7] - C_a * D[3] + C_f * D[5];

    D[0] = C_norm * (Y04P2b6ePP + Y7f1aP3c5dPP);
    D[7] = C_norm * (Y04P2b6ePP - Y7f1aP3c5dPP);
    D[4] = C_norm * (Y04P2b6ePM + Y7a1fM3d5cMP);
    D[3] = C_norm * (Y04P2b6ePM - Y7a1fM3d5cMP);

    D[1] = C_norm * (Y04M2e6bMP + Y1c7dM3f5aPM);
    D[5] = C_norm * (Y04M2e6bMM - Y1d7cP3a5fMM);
    D[2] = C_norm * (Y04M2e6bMM + Y1d7cP3a5fMM);
    D[6] = C_norm * (Y04M2e6bMP - Y1c7dM3f5aPM);
}
extern "C"

void DCT8x8_kernel(
    float* d_Dst,
    const float* d_Src,
    const unsigned int stride,
    const unsigned int imageH,
    const unsigned int imageW
){
    #pragma HLS INTERFACE m_axi port=d_Dst offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_Src offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=stride
    #pragma HLS INTERFACE s_axilite port=imageH
    #pragma HLS INTERFACE s_axilite port=imageW
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=l_Transpose complete dim=1
    #pragma HLS ARRAY_PARTITION variable=l_Transpose complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    const unsigned int    localX = _tid_x;
                    const unsigned int    localY = BLOCK_SIZE * _tid_y;
                    const unsigned int modLocalX = localX & (BLOCK_SIZE - 1);
                    const unsigned int   globalX = _bid_x * BLOCK_X + localX;
                    const unsigned int   globalY = _bid_y * BLOCK_Y + localY;

                    float l_Transpose[BLOCK_Y * (BLOCK_X+1)];

                    //Process only full blocks
                    if( (globalX - modLocalX + BLOCK_SIZE - 1 >= imageW) || (globalY + BLOCK_SIZE - 1 >= imageH) )
                    return;

                    float *l_V = &l_Transpose[localY * (BLOCK_X+1) + localX];
                    float *l_H = &l_Transpose[(localY + modLocalX) * (BLOCK_X+1) + localX - modLocalX];
                    d_Src += globalY * stride + globalX;
                    d_Dst += globalY * stride + globalX;

                    float D[8];
                    for(unsigned int i = 0; i < BLOCK_SIZE; i++)
                    l_V[i * (BLOCK_X + 1)] = d_Src[i * stride];

                    for(unsigned int i = 0; i < BLOCK_SIZE; i++)
                    D[i] = l_H[i];
                    DCT8(D);
                    for(unsigned int i = 0; i < BLOCK_SIZE; i++)
                    l_H[i] = D[i];

                    for(unsigned int i = 0; i < BLOCK_SIZE; i++)
                    D[i] = l_V[i * (BLOCK_X + 1)];
                    DCT8(D);

                    for(unsigned int i = 0; i < BLOCK_SIZE; i++)
                    d_Dst[i * stride] = D[i];

                }
            }
        }
    }
}
extern "C"

void IDCT8x8_kernel(
    float* d_Dst,
    const float* d_Src,
    const unsigned int stride,
    const unsigned int imageH,
    const unsigned int imageW
){
    #pragma HLS INTERFACE m_axi port=d_Dst offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_Src offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=stride
    #pragma HLS INTERFACE s_axilite port=imageH
    #pragma HLS INTERFACE s_axilite port=imageW
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=l_Transpose complete dim=1
    #pragma HLS ARRAY_PARTITION variable=l_Transpose complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    const unsigned int    localX = _tid_x;
                    const unsigned int    localY = BLOCK_SIZE * _tid_y;
                    const unsigned int modLocalX = localX & (BLOCK_SIZE - 1);
                    const unsigned int   globalX = _bid_x * BLOCK_X + localX;
                    const unsigned int   globalY = _bid_y * BLOCK_Y + localY;

                    float l_Transpose[BLOCK_Y * (BLOCK_X+1)];

                    //Process only full blocks
                    if( (globalX - modLocalX + BLOCK_SIZE - 1 >= imageW) || (globalY + BLOCK_SIZE - 1 >= imageH) )
                    return;

                    float *l_V = &l_Transpose[localY * (BLOCK_X+1) + localX];
                    float *l_H = &l_Transpose[(localY + modLocalX) * (BLOCK_X+1) + localX - modLocalX];
                    d_Src += globalY * stride + globalX;
                    d_Dst += globalY * stride + globalX;

                    float D[8];
                    for(unsigned int i = 0; i < BLOCK_SIZE; i++)
                    l_V[i * (BLOCK_X + 1)] = d_Src[i * stride];

                    for(unsigned int i = 0; i < BLOCK_SIZE; i++)
                    D[i] = l_H[i];
                    IDCT8(D);
                    for(unsigned int i = 0; i < BLOCK_SIZE; i++)
                    l_H[i] = D[i];

                    for(unsigned int i = 0; i < BLOCK_SIZE; i++)
                    D[i] = l_V[i * (BLOCK_X + 1)];
                    IDCT8(D);
                    for(unsigned int i = 0; i < BLOCK_SIZE; i++)
                    d_Dst[i * stride] = D[i];

                }
            }
        }
    }
}
