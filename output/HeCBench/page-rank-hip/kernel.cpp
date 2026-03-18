#include "kernel.h"

// --- from main.cu ---
extern "C"
void map(const int * pages,
         const float * page_ranks,
               float * maps,
         const unsigned int * noutlinks,
         const int n)
{
    #pragma HLS INTERFACE m_axi port=pages offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=page_ranks offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=maps offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=noutlinks offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _tid_x + _bid_x * BLOCK_DIM_X;
            int j;
            if(i < n){
            float outbound_rank = page_ranks[i]/(float)noutlinks[i];
            for(j=0; j<n; ++j){
            maps[(size_t)i*n+j] = pages[(size_t)i*n+j]*outbound_rank;
            }
            }

        }
    }
}
extern "C"

void reduce(      float * page_ranks,
            const float * maps,
            const int n,
                  float * dif)
{
    #pragma HLS INTERFACE m_axi port=page_ranks offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=maps offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=dif offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            int j = _tid_x + _bid_x * BLOCK_DIM_X;
            int i;
            float new_rank;
            float old_rank;

            if(j<n){
            old_rank = page_ranks[j];
            new_rank = 0.0f;
            for(i=0; i< n; ++i){
            new_rank += maps[(size_t)i*n + j];
            }

            new_rank = ((1.f-D_FACTOR)/n)+(D_FACTOR*new_rank);
            dif[j] = fmaxf(fabsf(new_rank - old_rank), dif[j]);
            page_ranks[j] = new_rank;
            }

        }
    }
}
