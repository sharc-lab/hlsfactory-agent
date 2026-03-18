#include "kernel.h"

// --- from kernels.cu ---
extern "C"
void hypterm_1 (double *  flux_0,
                double *  flux_1,
                double *  flux_2,
                double *  flux_3,
                double *  flux_4,
                const double *  cons_1,
                const double *  cons_2,
                const double *  cons_3,
                const double *  cons_4, 
                const double *  q_1,
                const double *  q_2,
                const double *  q_3,
                const double *  q_4,
                double dxinv0, double dxinv1, double dxinv2,
                int L, int M, int N)
{
    #pragma HLS INTERFACE m_axi port=flux_0 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=flux_1 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=flux_2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=flux_3 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=flux_4 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=cons_1 offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=cons_2 offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=cons_3 offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=cons_4 offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=q_1 offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=q_2 offset=slave bundle=gmem10
    #pragma HLS INTERFACE m_axi port=q_3 offset=slave bundle=gmem11
    #pragma HLS INTERFACE m_axi port=q_4 offset=slave bundle=gmem12
    #pragma HLS INTERFACE s_axilite port=dxinv0
    #pragma HLS INTERFACE s_axilite port=dxinv1
    #pragma HLS INTERFACE s_axilite port=dxinv2
    #pragma HLS INTERFACE s_axilite port=L
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            //Determing the block's indices
                            int blockdim_i= (int)(BLOCK_DIM_X);
                            int i0 = (int)(_bid_x)*(blockdim_i);
                            int i = max (i0, 0) + (int)(_tid_x);
                            int blockdim_j= (int)(BLOCK_DIM_Y);
                            int j0 = (int)(_bid_y)*(blockdim_j);
                            int j = max (j0, 0) + (int)(_tid_y);
                            int blockdim_k= (int)(BLOCK_DIM_Z);
                            int k0 = (int)(_bid_z)*(blockdim_k);
                            int k = max (k0, 0) + (int)(_tid_z);

                            if (i>=4 & j>=4 & k>=4 & i<=N-5 & j<=N-5 & k<=N-5) {
                            flux_0[k*M*N+j*N+i] = -((0.8f*(cons_1[k*M*N+j*N+i+1]-cons_1[k*M*N+j*N+i-1])-0.2f*(cons_1[k*M*N+j*N+i+2]-cons_1[k*M*N+j*N+i-2])+0.038f*(cons_1[k*M*N+j*N+i+3]-cons_1[k*M*N+j*N+i-3])-0.0035f*(cons_1[k*M*N+j*N+i+4]-cons_1[k*M*N+j*N+i-4]))*dxinv0);
                            flux_1[k*M*N+j*N+i] = -((0.8f*(cons_1[k*M*N+j*N+i+1]*q_1[k*M*N+j*N+i+1]-cons_1[k*M*N+j*N+i-1]*q_1[k*M*N+j*N+i-1]+(q_4[k*M*N+j*N+i+1]-q_4[k*M*N+j*N+i-1]))-0.2f*(cons_1[k*M*N+j*N+i+2]*q_1[k*M*N+j*N+i+2]-cons_1[k*M*N+j*N+i-2]*q_1[k*M*N+j*N+i-2]+(q_4[k*M*N+j*N+i+2]-q_4[k*M*N+j*N+i-2]))+0.038f*(cons_1[k*M*N+j*N+i+3]*q_1[k*M*N+j*N+i+3]-cons_1[k*M*N+j*N+i-3]*q_1[k*M*N+j*N+i-3]+(q_4[k*M*N+j*N+i+3]-q_4[k*M*N+j*N+i-3]))-0.0035f*(cons_1[k*M*N+j*N+i+4]*q_1[k*M*N+j*N+i+4]-cons_1[k*M*N+j*N+i-4]*q_1[k*M*N+j*N+i-4]+(q_4[k*M*N+j*N+i+4]-q_4[k*M*N+j*N+i-4])))*dxinv0);
                            flux_2[k*M*N+j*N+i] = -((0.8f*(cons_2[k*M*N+j*N+i+1]*q_1[k*M*N+j*N+i+1]-cons_2[k*M*N+j*N+i-1]*q_1[k*M*N+j*N+i-1])-0.2f*(cons_2[k*M*N+j*N+i+2]*q_1[k*M*N+j*N+i+2]-cons_2[k*M*N+j*N+i-2]*q_1[k*M*N+j*N+i-2])+0.038f*(cons_2[k*M*N+j*N+i+3]*q_1[k*M*N+j*N+i+3]-cons_2[k*M*N+j*N+i-3]*q_1[k*M*N+j*N+i-3])-0.0035f*(cons_2[k*M*N+j*N+i+4]*q_1[k*M*N+j*N+i+4]-cons_2[k*M*N+j*N+i-4]*q_1[k*M*N+j*N+i-4]))*dxinv0);
                            flux_3[k*M*N+j*N+i] = -((0.8f*(cons_3[k*M*N+j*N+i+1]*q_1[k*M*N+j*N+i+1]-cons_3[k*M*N+j*N+i-1]*q_1[k*M*N+j*N+i-1])-0.2f*(cons_3[k*M*N+j*N+i+2]*q_1[k*M*N+j*N+i+2]-cons_3[k*M*N+j*N+i-2]*q_1[k*M*N+j*N+i-2])+0.038f*(cons_3[k*M*N+j*N+i+3]*q_1[k*M*N+j*N+i+3]-cons_3[k*M*N+j*N+i-3]*q_1[k*M*N+j*N+i-3])-0.0035f*(cons_3[k*M*N+j*N+i+4]*q_1[k*M*N+j*N+i+4]-cons_3[k*M*N+j*N+i-4]*q_1[k*M*N+j*N+i-4]))*dxinv0);
                            flux_4[k*M*N+j*N+i] = -((0.8f*(cons_4[k*M*N+j*N+i+1]*q_1[k*M*N+j*N+i+1]-cons_4[k*M*N+j*N+i-1]*q_1[k*M*N+j*N+i-1]+(q_4[k*M*N+j*N+i+1]*q_1[k*M*N+j*N+i+1]-q_4[k*M*N+j*N+i-1]*q_1[k*M*N+j*N+i-1]))-0.2f*(cons_4[k*M*N+j*N+i+2]*q_1[k*M*N+j*N+i+2]-cons_4[k*M*N+j*N+i-2]*q_1[k*M*N+j*N+i-2]+(q_4[k*M*N+j*N+i+2]*q_1[k*M*N+j*N+i+2]-q_4[k*M*N+j*N+i-2]*q_1[k*M*N+j*N+i-2]))+0.038f*(cons_4[k*M*N+j*N+i+3]*q_1[k*M*N+j*N+i+3]-cons_4[k*M*N+j*N+i-3]*q_1[k*M*N+j*N+i-3]+(q_4[k*M*N+j*N+i+3]*q_1[k*M*N+j*N+i+3]-q_4[k*M*N+j*N+i-3]*q_1[k*M*N+j*N+i-3]))-0.0035f*(cons_4[k*M*N+j*N+i+4]*q_1[k*M*N+j*N+i+4]-cons_4[k*M*N+j*N+i-4]*q_1[k*M*N+j*N+i-4]+(q_4[k*M*N+j*N+i+4]*q_1[k*M*N+j*N+i+4]-q_4[k*M*N+j*N+i-4]*q_1[k*M*N+j*N+i-4])))*dxinv0);
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void hypterm_2 (double *  flux_0,
                double *  flux_1,
                double *  flux_2,
                double *  flux_3,
                double *  flux_4,
                const double *  cons_1,
                const double *  cons_2,
                const double *  cons_3,
                const double *  cons_4, 
                const double *  q_1,
                const double *  q_2,
                const double *  q_3,
                const double *  q_4,
                double dxinv0, double dxinv1, double dxinv2,
                int L, int M, int N)
{
    #pragma HLS INTERFACE m_axi port=flux_0 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=flux_1 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=flux_2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=flux_3 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=flux_4 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=cons_1 offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=cons_2 offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=cons_3 offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=cons_4 offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=q_1 offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=q_2 offset=slave bundle=gmem10
    #pragma HLS INTERFACE m_axi port=q_3 offset=slave bundle=gmem11
    #pragma HLS INTERFACE m_axi port=q_4 offset=slave bundle=gmem12
    #pragma HLS INTERFACE s_axilite port=dxinv0
    #pragma HLS INTERFACE s_axilite port=dxinv1
    #pragma HLS INTERFACE s_axilite port=dxinv2
    #pragma HLS INTERFACE s_axilite port=L
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            //Determing the block's indices
                            int blockdim_i= (int)(BLOCK_DIM_X);
                            int i0 = (int)(_bid_x)*(blockdim_i);
                            int i = max (i0, 0) + (int)(_tid_x);
                            int blockdim_j= (int)(BLOCK_DIM_Y);
                            int j0 = (int)(_bid_y)*(blockdim_j);
                            int j = max (j0, 0) + (int)(_tid_y);
                            int blockdim_k= (int)(BLOCK_DIM_Z);
                            int k0 = (int)(_bid_z)*(blockdim_k);
                            int k = max (k0, 0) + (int)(_tid_z);

                            if (i>=4 & j>=4 & k>=4 & i<=N-5 & j<=N-5 & k<=N-5) {
                            flux_0[k*M*N+j*N+i] -= (0.8f*(cons_2[k*M*N+(j+1)*N+i]-cons_2[k*M*N+(j-1)*N+i])-0.2f*(cons_2[k*M*N+(j+2)*N+i]-cons_2[k*M*N+(j-2)*N+i])+0.038f*(cons_2[k*M*N+(j+3)*N+i]-cons_2[k*M*N+(j-3)*N+i])-0.0035f*(cons_2[k*M*N+(j+4)*N+i]-cons_2[k*M*N+(j-4)*N+i]))*dxinv1;
                            flux_1[k*M*N+j*N+i] -= (0.8f*(cons_1[k*M*N+(j+1)*N+i]*q_2[k*M*N+(j+1)*N+i]-cons_1[k*M*N+(j-1)*N+i]*q_2[k*M*N+(j-1)*N+i])-0.2f*(cons_1[k*M*N+(j+2)*N+i]*q_2[k*M*N+(j+2)*N+i]-cons_1[k*M*N+(j-2)*N+i]*q_2[k*M*N+(j-2)*N+i])+0.038f*(cons_1[k*M*N+(j+3)*N+i]*q_2[k*M*N+(j+3)*N+i]-cons_1[k*M*N+(j-3)*N+i]*q_2[k*M*N+(j-3)*N+i])-0.0035f*(cons_1[k*M*N+(j+4)*N+i]*q_2[k*M*N+(j+4)*N+i]-cons_1[k*M*N+(j-4)*N+i]*q_2[k*M*N+(j-4)*N+i]))*dxinv1;
                            flux_2[k*M*N+j*N+i] -= (0.8f*(cons_2[k*M*N+(j+1)*N+i]*q_2[k*M*N+(j+1)*N+i]-cons_2[k*M*N+(j-1)*N+i]*q_2[k*M*N+(j-1)*N+i]+(q_4[k*M*N+(j+1)*N+i]-q_4[k*M*N+(j-1)*N+i]))-0.2f*(cons_2[k*M*N+(j+2)*N+i]*q_2[k*M*N+(j+2)*N+i]-cons_2[k*M*N+(j-2)*N+i]*q_2[k*M*N+(j-2)*N+i]+(q_4[k*M*N+(j+2)*N+i]-q_4[k*M*N+(j-2)*N+i]))+0.038f*(cons_2[k*M*N+(j+3)*N+i]*q_2[k*M*N+(j+3)*N+i]-cons_2[k*M*N+(j-3)*N+i]*q_2[k*M*N+(j-3)*N+i]+(q_4[k*M*N+(j+3)*N+i]-q_4[k*M*N+(j-3)*N+i]))-0.0035f*(cons_2[k*M*N+(j+4)*N+i]*q_2[k*M*N+(j+4)*N+i]-cons_2[k*M*N+(j-4)*N+i]*q_2[k*M*N+(j-4)*N+i]+(q_4[k*M*N+(j+4)*N+i]-q_4[k*M*N+(j-4)*N+i])))*dxinv1;
                            flux_3[k*M*N+j*N+i] -= (0.8f*(cons_3[k*M*N+(j+1)*N+i]*q_2[k*M*N+(j+1)*N+i]-cons_3[k*M*N+(j-1)*N+i]*q_2[k*M*N+(j-1)*N+i])-0.2f*(cons_3[k*M*N+(j+2)*N+i]*q_2[k*M*N+(j+2)*N+i]-cons_3[k*M*N+(j-2)*N+i]*q_2[k*M*N+(j-2)*N+i])+0.038f*(cons_3[k*M*N+(j+3)*N+i]*q_2[k*M*N+(j+3)*N+i]-cons_3[k*M*N+(j-3)*N+i]*q_2[k*M*N+(j-3)*N+i])-0.0035f*(cons_3[k*M*N+(j+4)*N+i]*q_2[k*M*N+(j+4)*N+i]-cons_3[k*M*N+(j-4)*N+i]*q_2[k*M*N+(j-4)*N+i]))*dxinv1;
                            flux_4[k*M*N+j*N+i] -= (0.8f*(cons_4[(k+1)*M*N+j*N+i]*q_3[(k+1)*M*N+j*N+i]-cons_4[(k-1)*M*N+j*N+i]*q_3[(k-1)*M*N+j*N+i]+(q_4[(k+1)*M*N+j*N+i]*q_3[(k+1)*M*N+j*N+i]-q_4[(k-1)*M*N+j*N+i]*q_3[(k-1)*M*N+j*N+i]))-0.2f*(cons_4[(k+2)*M*N+j*N+i]*q_3[(k+2)*M*N+j*N+i]-cons_4[(k-2)*M*N+j*N+i]*q_3[(k-2)*M*N+j*N+i]+(q_4[(k+2)*M*N+j*N+i]*q_3[(k+2)*M*N+j*N+i]-q_4[(k-2)*M*N+j*N+i]*q_3[(k-2)*M*N+j*N+i]))+0.038f*(cons_4[(k+3)*M*N+j*N+i]*q_3[(k+3)*M*N+j*N+i]-cons_4[(k-3)*M*N+j*N+i]*q_3[(k-3)*M*N+j*N+i]+(q_4[(k+3)*M*N+j*N+i]*q_3[(k+3)*M*N+j*N+i]-q_4[(k-3)*M*N+j*N+i]*q_3[(k-3)*M*N+j*N+i]))-0.0035f*(cons_4[(k+4)*M*N+j*N+i]*q_3[(k+4)*M*N+j*N+i]-cons_4[(k-4)*M*N+j*N+i]*q_3[(k-4)*M*N+j*N+i]+(q_4[(k+4)*M*N+j*N+i]*q_3[(k+4)*M*N+j*N+i]-q_4[(k-4)*M*N+j*N+i]*q_3[(k-4)*M*N+j*N+i])))*dxinv2;
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void hypterm_3 (double *  flux_0,
                double *  flux_1,
                double *  flux_2,
                double *  flux_3,
                double *  flux_4,
                const double *  cons_1,
                const double *  cons_2,
                const double *  cons_3,
                const double *  cons_4, 
                const double *  q_1,
                const double *  q_2,
                const double *  q_3,
                const double *  q_4,
                double dxinv0, double dxinv1, double dxinv2,
                int L, int M, int N)
{
    #pragma HLS INTERFACE m_axi port=flux_0 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=flux_1 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=flux_2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=flux_3 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=flux_4 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=cons_1 offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=cons_2 offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=cons_3 offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=cons_4 offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=q_1 offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=q_2 offset=slave bundle=gmem10
    #pragma HLS INTERFACE m_axi port=q_3 offset=slave bundle=gmem11
    #pragma HLS INTERFACE m_axi port=q_4 offset=slave bundle=gmem12
    #pragma HLS INTERFACE s_axilite port=dxinv0
    #pragma HLS INTERFACE s_axilite port=dxinv1
    #pragma HLS INTERFACE s_axilite port=dxinv2
    #pragma HLS INTERFACE s_axilite port=L
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            //Determing the block's indices
                            int blockdim_i= (int)(BLOCK_DIM_X);
                            int i0 = (int)(_bid_x)*(blockdim_i);
                            int i = max (i0, 0) + (int)(_tid_x);
                            int blockdim_j= (int)(BLOCK_DIM_Y);
                            int j0 = (int)(_bid_y)*(blockdim_j);
                            int j = max (j0, 0) + (int)(_tid_y);
                            int blockdim_k= (int)(BLOCK_DIM_Z);
                            int k0 = (int)(_bid_z)*(blockdim_k);
                            int k = max (k0, 0) + (int)(_tid_z);

                            if (i>=4 & j>=4 & k>=4 & i<=N-5 & j<=N-5 & k<=N-5) {
                            flux_0[k*M*N+j*N+i] -= (0.8f*(cons_3[(k+1)*M*N+j*N+i]-cons_3[(k-1)*M*N+j*N+i])-0.2f*(cons_3[(k+2)*M*N+j*N+i]-cons_3[(k-2)*M*N+j*N+i])+0.038f*(cons_3[(k+3)*M*N+j*N+i]-cons_3[(k-3)*M*N+j*N+i])-0.0035f*(cons_3[(k+4)*M*N+j*N+i]-cons_3[(k-4)*M*N+j*N+i]))*dxinv2;
                            flux_1[k*M*N+j*N+i] -= (0.8f*(cons_1[(k+1)*M*N+j*N+i]*q_3[(k+1)*M*N+j*N+i]-cons_1[(k-1)*M*N+j*N+i]*q_3[(k-1)*M*N+j*N+i])-0.2f*(cons_1[(k+2)*M*N+j*N+i]*q_3[(k+2)*M*N+j*N+i]-cons_1[(k-2)*M*N+j*N+i]*q_3[(k-2)*M*N+j*N+i])+0.038f*(cons_1[(k+3)*M*N+j*N+i]*q_3[(k+3)*M*N+j*N+i]-cons_1[(k-3)*M*N+j*N+i]*q_3[(k-3)*M*N+j*N+i])-0.0035f*(cons_1[(k+4)*M*N+j*N+i]*q_3[(k+4)*M*N+j*N+i]-cons_1[(k-4)*M*N+j*N+i]*q_3[(k-4)*M*N+j*N+i]))*dxinv2;
                            flux_2[k*M*N+j*N+i] -= (0.8f*(cons_2[(k+1)*M*N+j*N+i]*q_3[(k+1)*M*N+j*N+i]-cons_2[(k-1)*M*N+j*N+i]*q_3[(k-1)*M*N+j*N+i])-0.2f*(cons_2[(k+2)*M*N+j*N+i]*q_3[(k+2)*M*N+j*N+i]-cons_2[(k-2)*M*N+j*N+i]*q_3[(k-2)*M*N+j*N+i])+0.038f*(cons_2[(k+3)*M*N+j*N+i]*q_3[(k+3)*M*N+j*N+i]-cons_2[(k-3)*M*N+j*N+i]*q_3[(k-3)*M*N+j*N+i])-0.0035f*(cons_2[(k+4)*M*N+j*N+i]*q_3[(k+4)*M*N+j*N+i]-cons_2[(k-4)*M*N+j*N+i]*q_3[(k-4)*M*N+j*N+i]))*dxinv2;
                            flux_3[k*M*N+j*N+i] -= (0.8f*(cons_3[(k+1)*M*N+j*N+i]*q_3[(k+1)*M*N+j*N+i]-cons_3[(k-1)*M*N+j*N+i]*q_3[(k-1)*M*N+j*N+i]+(q_4[(k+1)*M*N+j*N+i]-q_4[(k-1)*M*N+j*N+i]))-0.2f*(cons_3[(k+2)*M*N+j*N+i]*q_3[(k+2)*M*N+j*N+i]-cons_3[(k-2)*M*N+j*N+i]*q_3[(k-2)*M*N+j*N+i]+(q_4[(k+2)*M*N+j*N+i]-q_4[(k-2)*M*N+j*N+i]))+0.038f*(cons_3[(k+3)*M*N+j*N+i]*q_3[(k+3)*M*N+j*N+i]-cons_3[(k-3)*M*N+j*N+i]*q_3[(k-3)*M*N+j*N+i]+(q_4[(k+3)*M*N+j*N+i]-q_4[(k-3)*M*N+j*N+i]))-0.0035f*(cons_3[(k+4)*M*N+j*N+i]*q_3[(k+4)*M*N+j*N+i]-cons_3[(k-4)*M*N+j*N+i]*q_3[(k-4)*M*N+j*N+i]+(q_4[(k+4)*M*N+j*N+i]-q_4[(k-4)*M*N+j*N+i])))*dxinv2;
                            flux_4[k*M*N+j*N+i] -= (0.8f*(cons_4[k*M*N+(j+1)*N+i]*q_2[k*M*N+(j+1)*N+i]-cons_4[k*M*N+(j-1)*N+i]*q_2[k*M*N+(j-1)*N+i]+(q_4[k*M*N+(j+1)*N+i]*q_2[k*M*N+(j+1)*N+i]-q_4[k*M*N+(j-1)*N+i]*q_2[k*M*N+(j-1)*N+i]))-0.2f*(cons_4[k*M*N+(j+2)*N+i]*q_2[k*M*N+(j+2)*N+i]-cons_4[k*M*N+(j-2)*N+i]*q_2[k*M*N+(j-2)*N+i]+(q_4[k*M*N+(j+2)*N+i]*q_2[k*M*N+(j+2)*N+i]-q_4[k*M*N+(j-2)*N+i]*q_2[k*M*N+(j-2)*N+i]))+0.038f*(cons_4[k*M*N+(j+3)*N+i]*q_2[k*M*N+(j+3)*N+i]-cons_4[k*M*N+(j-3)*N+i]*q_2[k*M*N+(j-3)*N+i]+(q_4[k*M*N+(j+3)*N+i]*q_2[k*M*N+(j+3)*N+i]-q_4[k*M*N+(j-3)*N+i]*q_2[k*M*N+(j-3)*N+i]))-0.0035f*(cons_4[k*M*N+(j+4)*N+i]*q_2[k*M*N+(j+4)*N+i]-cons_4[k*M*N+(j-4)*N+i]*q_2[k*M*N+(j-4)*N+i]+(q_4[k*M*N+(j+4)*N+i]*q_2[k*M*N+(j+4)*N+i]-q_4[k*M*N+(j-4)*N+i]*q_2[k*M*N+(j-4)*N+i])))*dxinv1;
                            }

                        }
                    }
                }
            }
        }
    }
}
