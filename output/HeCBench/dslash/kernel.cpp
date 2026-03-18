#include "kernel.h"

// --- from kernels.cu ---
extern "C"
void make_back(
    const su3_matrix* d_fat,
    const su3_matrix* d_lng,
    const size_t* d_bck, 
    const size_t* d_bck3,
          su3_matrix* d_fatbck,
          su3_matrix* d_lngbck,
    const int total_even_sites)
{
    #pragma HLS INTERFACE m_axi port=d_fat offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_lng offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_bck offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_bck3 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=d_fatbck offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=d_lngbck offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=total_even_sites
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            size_t mySite = _bid_x * BLOCK_DIM_X + _tid_x;
            if (mySite < total_even_sites) {
            for(int dir = 0; dir < 4; dir++) {
            su3_adjoint( d_fat + 4*d_bck[4*mySite+dir]+dir,
            d_fatbck + 4*mySite+dir );
            su3_adjoint( d_lng + 4*d_bck3[4*mySite+dir]+dir,
            d_lngbck + 4*mySite+dir );
            }
            }

        }
    }
}
extern "C"

void dslash (
    const su3_matrix* d_fat,
    const su3_matrix* d_lng,
    const su3_matrix* d_fatbck,
    const su3_matrix* d_lngbck,
    const su3_vector* d_src,
          su3_vector* d_dst,
    const size_t* d_fwd,
    const size_t* d_bck,
    const size_t* d_fwd3,
    const size_t* d_bck3,
    const int total_even_sites)
{
    #pragma HLS INTERFACE m_axi port=d_fat offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_lng offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_fatbck offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_lngbck offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=d_src offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=d_dst offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=d_fwd offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=d_bck offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=d_fwd3 offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=d_bck3 offset=slave bundle=gmem9
    #pragma HLS INTERFACE s_axilite port=total_even_sites
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            size_t mySite = _bid_x * BLOCK_DIM_X + _tid_x;
            if (mySite < total_even_sites) {
            su3_vector v;
            for (size_t k=0; k<4; ++k) {
            auto a = d_fat + mySite*4 + k;
            auto b = d_src + d_fwd[4*mySite + k];
            if (k == 0)
            mult_su3_mat_vec(a, b, &d_dst[mySite]);
            else
            mult_su3_mat_vec_sum(a, b, &d_dst[mySite]);
            }
            for (size_t k=0; k<4; ++k) {
            auto a = d_lng + mySite*4 + k;
            auto b = d_src + d_fwd3[4*mySite + k];
            if (k == 0)
            mult_su3_mat_vec(a, b, &v);
            else
            mult_su3_mat_vec_sum(a, b, &v);
            }
            add_su3_vector(&d_dst[mySite], &v, &d_dst[mySite]);
            for (size_t k=0; k<4; ++k) {
            auto a = d_fatbck + mySite*4 + k;
            auto b = d_src + d_bck[4*mySite + k];
            if (k == 0)
            mult_su3_mat_vec(a, b, &v);
            else
            mult_su3_mat_vec_sum(a, b, &v);
            }
            sub_su3_vector(&d_dst[mySite], &v, &d_dst[mySite]);
            for (size_t k=0; k<4; ++k) {
            auto a = d_lngbck + mySite*4 + k;
            auto b = d_src + d_bck3[4*mySite + k];
            if (k == 0)
            mult_su3_mat_vec(a, b, &v);
            else
            mult_su3_mat_vec_sum(a, b, &v);
            }
            sub_su3_vector(&d_dst[mySite], &v, &d_dst[mySite]);
            } // end of if mySite

        }
    }
}
