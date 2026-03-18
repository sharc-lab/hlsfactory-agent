#include "kernel.h"

// --- from kernel.cu ---
extern "C"
void ldpc_cnp_kernel_1st_iter(
    const float * dev_llr,
    float * dev_dt,
    float * dev_R,
    const char * dev_h_element_count1,
    const h_element * dev_h_compact1)
{
    #pragma HLS INTERFACE m_axi port=dev_llr offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dev_dt offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dev_R offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=dev_h_element_count1 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=dev_h_compact1 offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    #if MODE == WIFI
                    if(_tid_x >= Z)
                    return;
                    #endif

                    int iCW = _tid_y; // index of CW in a MCW
                    int iMCW = _bid_y; // index of MCW
                    int iCurrentCW = iMCW * CW + iCW;

                    //for step 1: update dt
                    int iBlkRow; // block row in h_base
                    int iBlkCol; // block col in h_base
                    int iSubRow; // row index in sub_block of h_base
                    int iCol; // overall col index in h_base
                    int offsetR;

                    iSubRow = _tid_x;
                    iBlkRow = _bid_x;

                    int size_llr_CW = COL; // size of one llr CW block
                    int size_R_CW = ROW * BLK_COL;  // size of one R/dt CW block
                    int shift_t;

                    // For 2-min algorithm.
                    char Q_sign = 0;
                    char sq;
                    float Q, Q_abs;
                    float R_temp;

                    float sign = 1.0f;
                    float rmin1 = 1000.0f;
                    float rmin2 = 1000.0f;
                    char idx_min = 0;

                    h_element h_element_t;
                    int s = dev_h_element_count1[iBlkRow];
                    offsetR = size_R_CW * iCurrentCW + iBlkRow * Z + iSubRow;

                    // The 1st recursion
                    for(int i = 0; i < s; i++) // loop through all the ZxZ sub-blocks in a row
                    {
                    h_element_t = dev_h_compact1[i * H_COMPACT1_ROW + iBlkRow];

                    iBlkCol = h_element_t.y;
                    shift_t = h_element_t.value;

                    shift_t = (iSubRow + shift_t);
                    if(shift_t >= Z) shift_t = shift_t - Z;

                    iCol = iBlkCol * Z + shift_t;

                    Q = dev_llr[size_llr_CW * iCurrentCW + iCol];// - R_temp;
                    Q_abs = fabsf(Q);
                    sq = Q < 0;

                    // quick version
                    sign = sign * (1 - sq * 2);
                    Q_sign |= sq << i;

                    if (Q_abs < rmin1)
                    {
                    rmin2 = rmin1;
                    rmin1 = Q_abs;
                    idx_min = i;
                    } else if (Q_abs < rmin2)
                    {
                    rmin2 = Q_abs;
                    }
                    }

                    // The 2nd recursion
                    for(int i = 0; i < s; i ++)
                    {
                    // v0: Best performance so far. 0.75f is the value of alpha.
                    sq = 1 - 2 * ((Q_sign >> i) & 0x01);
                    R_temp = 0.75f * sign * sq * (i != idx_min ? rmin1 : rmin2);

                    // write results to global memory
                    h_element_t = dev_h_compact1[i * H_COMPACT1_ROW + iBlkRow];
                    int addr_temp = offsetR + h_element_t.y * ROW;
                    dev_dt[addr_temp] = R_temp;// - R1[i]; // compute the dt value for current llr.
                    dev_R[addr_temp] = R_temp; // update R, R=R'.
                    }

                }
            }
        }
    }
}
extern "C"

void ldpc_cnp_kernel(
    const float * dev_llr,
    float * dev_dt,
    float * dev_R,
    const char * dev_h_element_count1,
    const h_element * dev_h_compact1)
{
    #pragma HLS INTERFACE m_axi port=dev_llr offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dev_dt offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dev_R offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=dev_h_element_count1 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=dev_h_compact1 offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=RCache complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    #if MODE == WIFI
                    if(_tid_x >= Z)
                    return;
                    #endif

                    // Define cache for R: Rcache[NON_EMPTY_ELMENT][nThreadPerBlock]
                    // extern means that the memory is allocated dynamically at run-time
                    float RCache[4096];
                    int iRCacheLine = _tid_y * BLOCK_DIM_X + _tid_x;

                    int iCW = _tid_y; // index of CW in a MCW
                    int iMCW = _bid_y; // index of MCW
                    int iCurrentCW = iMCW * CW + iCW;

                    //for step 1: update dt
                    int iBlkRow; // block row in h_base
                    int iBlkCol; // block col in h_base
                    int iSubRow; // row index in sub_block of h_base
                    int iCol; // overall col index in h_base
                    int offsetR;

                    iSubRow = _tid_x;
                    iBlkRow = _bid_x;

                    int size_llr_CW = COL; // size of one llr CW block
                    int size_R_CW = ROW * BLK_COL;  // size of one R/dt CW block

                    //float R1[NON_EMPTY_ELMENT];
                    int shift_t;

                    // For 2-min algorithm.
                    char Q_sign = 0;
                    char sq;
                    float Q, Q_abs;
                    float R_temp;

                    float sign = 1.0f;
                    float rmin1 = 1000.0f;
                    float rmin2 = 1000.0f;
                    char idx_min = 0;

                    h_element h_element_t;
                    int s = dev_h_element_count1[iBlkRow];
                    offsetR = size_R_CW * iCurrentCW + iBlkRow * Z + iSubRow;

                    // The 1st recursion
                    // TODO: Is s always the same? If so we can unroll the loop with #pragma unroll
                    for(int i = 0; i < s; i++) // loop through all the ZxZ sub-blocks in a row
                    {
                    h_element_t = dev_h_compact1[i * H_COMPACT1_ROW + iBlkRow];

                    iBlkCol = h_element_t.y;
                    shift_t = h_element_t.value;

                    shift_t = (iSubRow + shift_t);
                    if(shift_t >= Z) shift_t = shift_t - Z;

                    iCol = iBlkCol * Z + shift_t;

                    R_temp = dev_R[offsetR + iBlkCol * ROW];

                    RCache[i * THREADS_PER_BLOCK + iRCacheLine] =  R_temp;

                    Q = dev_llr[size_llr_CW * iCurrentCW + iCol] - R_temp;
                    Q_abs = fabsf(Q);

                    sq = Q < 0;
                    sign = sign * (1 - sq * 2);
                    Q_sign |= sq << i;

                    if (Q_abs < rmin1)
                    {
                    rmin2 = rmin1;
                    rmin1 = Q_abs;
                    idx_min = i;
                    } else if (Q_abs < rmin2)
                    {
                    rmin2 = Q_abs;
                    }
                    }

                    // The 2nd recursion
                    //#pragma unroll
                    for(int i = 0; i < s; i ++)
                    {
                    sq = 1 - 2 * ((Q_sign >> i) & 0x01);
                    R_temp = 0.75f * sign * sq * (i != idx_min ? rmin1 : rmin2);

                    // write results to global memory
                    h_element_t = dev_h_compact1[i * H_COMPACT1_ROW + iBlkRow];
                    int addr_temp = h_element_t.y * ROW + offsetR;
                    dev_dt[addr_temp] = R_temp - RCache[i * THREADS_PER_BLOCK + iRCacheLine];
                    dev_R[addr_temp] = R_temp; // update R, R=R'.
                    }

                }
            }
        }
    }
}
extern "C"

  void ldpc_vnp_kernel_normal(
    float * dev_llr, 
    float * dev_dt, 
    const char *dev_h_element_count2,
    const h_element *dev_h_compact2)
{
    #pragma HLS INTERFACE m_axi port=dev_llr offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dev_dt offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dev_h_element_count2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=dev_h_compact2 offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1


                    #if MODE == WIFI
                    if(_tid_x >= Z)
                    return;
                    #endif

                    int  iCW = _tid_y; // index of CW in a MCW
                    int iMCW = _bid_y; // index of MCW
                    int iCurrentCW = iMCW * CW + iCW;

                    int iBlkCol;
                    int iBlkRow;
                    int iSubCol;
                    int iRow;
                    int iCol;

                    int shift_t, sf;
                    int llr_index;
                    float APP;

                    h_element h_element_t;

                    iBlkCol = _bid_x;
                    iSubCol = _tid_x;

                    int size_llr_CW = COL; // size of one llr CW block
                    int size_R_CW = ROW * BLK_COL;  // size of one R/dt CW block

                    // update all the llr values
                    iCol = iBlkCol * Z + iSubCol;
                    llr_index = size_llr_CW * iCurrentCW + iCol;

                    APP = dev_llr[llr_index];
                    int offsetDt = size_R_CW * iCurrentCW + iBlkCol * ROW;

                    for(int i = 0; i < dev_h_element_count2[iBlkCol]; i++)
                    {
                    h_element_t = dev_h_compact2[i * H_COMPACT2_COL + iBlkCol];

                    shift_t = h_element_t.value;
                    iBlkRow = h_element_t.x;

                    sf = iSubCol - shift_t;
                    if(sf < 0) sf = sf + Z;

                    iRow = iBlkRow * Z + sf;
                    APP = APP + dev_dt[offsetDt + iRow];
                    }
                    // write back to device global memory
                    dev_llr[llr_index] = APP;

                    // No hard decision for non-last iteration.

                }
            }
        }
    }
}
extern "C"

void ldpc_vnp_kernel_last_iter(
    const float * dev_llr,
    const float * dev_dt,
    int * dev_hd,
    const char *dev_h_element_count2,
    const h_element *dev_h_compact2)
{
    #pragma HLS INTERFACE m_axi port=dev_llr offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dev_dt offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dev_hd offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=dev_h_element_count2 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=dev_h_compact2 offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    #if MODE == WIFI
                    if(_tid_x >= Z)
                    return;
                    #endif

                    int  iCW = _tid_y; // index of CW in a MCW
                    int iMCW = _bid_y; // index of MCW
                    int iCurrentCW = iMCW * CW + iCW;

                    int iBlkCol;
                    int iBlkRow;
                    int iSubCol;
                    int iRow;
                    int iCol;

                    int shift_t, sf;
                    int llr_index;
                    float APP;

                    h_element h_element_t;

                    iBlkCol = _bid_x;
                    iSubCol = _tid_x;

                    int size_llr_CW = COL; // size of one llr CW block
                    int size_R_CW = ROW * BLK_COL;  // size of one R/dt CW block

                    // update all the llr values
                    iCol = iBlkCol * Z + iSubCol;
                    llr_index = size_llr_CW * iCurrentCW + iCol;

                    APP = dev_llr[llr_index];

                    int offsetDt = size_R_CW * iCurrentCW + iBlkCol * ROW;

                    for(int i = 0; i < dev_h_element_count2[iBlkCol]; i ++)
                    {
                    h_element_t = dev_h_compact2[i * H_COMPACT2_COL + iBlkCol];

                    shift_t = h_element_t.value;
                    iBlkRow = h_element_t.x;

                    sf = iSubCol - shift_t;
                    if(sf < 0) sf = sf + Z;

                    iRow = iBlkRow * Z + sf;
                    APP = APP + dev_dt[offsetDt + iRow];
                    }

                    // For the last iteration, we don't need to write intermediate results to
                    // global memory. Instead, we directly make a hard decision.
                    if(APP > 0)
                    dev_hd[llr_index] = 0;
                    else
                    dev_hd[llr_index] = 1;

                }
            }
        }
    }
}
