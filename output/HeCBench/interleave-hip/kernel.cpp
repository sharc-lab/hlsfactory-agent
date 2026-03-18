#include "kernel.h"

// --- from main.cu ---
extern "C"
void add_kernel_interleaved(
    INTERLEAVED_T * const dest_ptr,
    const INTERLEAVED_T * const src_ptr,
    const unsigned int num_elements)
{
    #pragma HLS INTERFACE m_axi port=dest_ptr offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=src_ptr offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=num_elements
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const unsigned int tid = _bid_x * BLOCK_DIM_X + _tid_x;
            if (tid < num_elements)
            {
            for (unsigned int i=0; i<COUNT; i++)
            {
            dest_ptr[tid].s0 += src_ptr[tid].s0;
            dest_ptr[tid].s1 += src_ptr[tid].s1;
            dest_ptr[tid].s2 += src_ptr[tid].s2;
            dest_ptr[tid].s3 += src_ptr[tid].s3;
            dest_ptr[tid].s4 += src_ptr[tid].s4;
            dest_ptr[tid].s5 += src_ptr[tid].s5;
            dest_ptr[tid].s6 += src_ptr[tid].s6;
            dest_ptr[tid].s7 += src_ptr[tid].s7;
            dest_ptr[tid].s8 += src_ptr[tid].s8;
            dest_ptr[tid].s9 += src_ptr[tid].s9;
            dest_ptr[tid].sa += src_ptr[tid].sa;
            dest_ptr[tid].sb += src_ptr[tid].sb;
            dest_ptr[tid].sc += src_ptr[tid].sc;
            dest_ptr[tid].sd += src_ptr[tid].sd;
            dest_ptr[tid].se += src_ptr[tid].se;
            dest_ptr[tid].sf += src_ptr[tid].sf;
            }
            }

        }
    }
}
extern "C"

void add_kernel_non_interleaved(
    NON_INTERLEAVED_T * const dest_ptr,
    const NON_INTERLEAVED_T * const src_ptr,
    const unsigned int num_elements)
{
    #pragma HLS INTERFACE m_axi port=dest_ptr offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=src_ptr offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=num_elements
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const unsigned int tid = _bid_x * BLOCK_DIM_X + _tid_x;
            if (tid < num_elements)
            {
            for (unsigned int i=0; i<COUNT; i++)
            {
            dest_ptr->s0[tid] += src_ptr->s0[tid];
            dest_ptr->s1[tid] += src_ptr->s1[tid];
            dest_ptr->s2[tid] += src_ptr->s2[tid];
            dest_ptr->s3[tid] += src_ptr->s3[tid];
            dest_ptr->s4[tid] += src_ptr->s4[tid];
            dest_ptr->s5[tid] += src_ptr->s5[tid];
            dest_ptr->s6[tid] += src_ptr->s6[tid];
            dest_ptr->s7[tid] += src_ptr->s7[tid];
            dest_ptr->s8[tid] += src_ptr->s8[tid];
            dest_ptr->s9[tid] += src_ptr->s9[tid];
            dest_ptr->sa[tid] += src_ptr->sa[tid];
            dest_ptr->sb[tid] += src_ptr->sb[tid];
            dest_ptr->sc[tid] += src_ptr->sc[tid];
            dest_ptr->sd[tid] += src_ptr->sd[tid];
            dest_ptr->se[tid] += src_ptr->se[tid];
            dest_ptr->sf[tid] += src_ptr->sf[tid];
            }
            }

        }
    }
}
