#include "kernel.h"

// --- from lud_kernels.cu ---
extern "C"
void lud_diagonal (float *m, const size_t matrix_dim, const int offset) {
    #pragma HLS INTERFACE m_axi port=m offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=matrix_dim
    #pragma HLS INTERFACE s_axilite port=offset
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shadow complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float shadow [BLOCK_SIZE*BLOCK_SIZE];
                    int i,j;
                    int tx = _tid_x;

                    size_t array_offset = offset * matrix_dim + offset;
                    for(i=0; i < BLOCK_SIZE; i++){
                    shadow[i * BLOCK_SIZE + tx]=m[array_offset + tx];
                    array_offset += matrix_dim;
                    }

                    for(i=0; i < BLOCK_SIZE-1; i++) {

                    if (tx>i){
                    for(j=0; j < i; j++)
                    shadow[tx * BLOCK_SIZE + i] -= shadow[tx * BLOCK_SIZE + j] * shadow[j * BLOCK_SIZE + i];
                    shadow[tx * BLOCK_SIZE + i] /= shadow[i * BLOCK_SIZE + i];
                    }
                    if (tx>i){

                    for(j=0; j < i+1; j++)
                    shadow[(i+1) * BLOCK_SIZE + tx] -= shadow[(i+1) * BLOCK_SIZE + j]*shadow[j * BLOCK_SIZE + tx];
                    }
                    }

                    array_offset = (offset+1)*matrix_dim+offset;
                    for(i=1; i < BLOCK_SIZE; i++){
                    m[array_offset+tx]=shadow[i * BLOCK_SIZE + tx];
                    array_offset += matrix_dim;
                    }

                }
            }
        }
    }
}
extern "C"

void lud_perimeter (float *m, const size_t matrix_dim, const int offset) {
    #pragma HLS INTERFACE m_axi port=m offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=matrix_dim
    #pragma HLS INTERFACE s_axilite port=offset
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=dia complete dim=1
    #pragma HLS ARRAY_PARTITION variable=peri_row complete dim=1
    #pragma HLS ARRAY_PARTITION variable=peri_col complete dim=1
    #pragma HLS ARRAY_PARTITION variable=peri_row complete dim=1
    #pragma HLS ARRAY_PARTITION variable=peri_col complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float dia [BLOCK_SIZE*BLOCK_SIZE];
                    float peri_row [BLOCK_SIZE*BLOCK_SIZE];
                    float peri_col [BLOCK_SIZE*BLOCK_SIZE];

                    size_t array_offset;
                    int i,j;
                    int idx;

                    int  bx = _bid_x;
                    int  tx = _tid_x;

                    if (tx < BLOCK_SIZE) {
                    idx = tx;
                    array_offset = offset*matrix_dim+offset;
                    for (i=0; i < BLOCK_SIZE/2; i++){
                    dia[i * BLOCK_SIZE + idx]=m[array_offset+idx];
                    array_offset += matrix_dim;
                    }

                    array_offset = offset*matrix_dim+offset;
                    for (i=0; i < BLOCK_SIZE; i++) {
                    peri_row[i * BLOCK_SIZE+ idx]=m[array_offset+(bx+1)*BLOCK_SIZE+idx];
                    array_offset += matrix_dim;
                    }

                    } else {
                    idx = tx-BLOCK_SIZE;

                    array_offset = (offset+BLOCK_SIZE/2)*matrix_dim+offset;
                    for (i=BLOCK_SIZE/2; i < BLOCK_SIZE; i++){
                    dia[i * BLOCK_SIZE + idx]=m[array_offset+idx];
                    array_offset += matrix_dim;
                    }

                    array_offset = (offset+(bx+1)*BLOCK_SIZE)*matrix_dim+offset;
                    for (i=0; i < BLOCK_SIZE; i++) {
                    peri_col[i * BLOCK_SIZE + idx] = m[array_offset+idx];
                    array_offset += matrix_dim;
                    }

                    }

                    if (tx < BLOCK_SIZE) { //peri-row
                    idx=tx;
                    for(i=1; i < BLOCK_SIZE; i++){
                    for (j=0; j < i; j++)
                    peri_row[i * BLOCK_SIZE + idx]-=dia[i * BLOCK_SIZE+ j]*peri_row[j * BLOCK_SIZE + idx];
                    }
                    } else { //peri-col
                    idx=tx - BLOCK_SIZE;
                    for(i=0; i < BLOCK_SIZE; i++){
                    for(j=0; j < i; j++)
                    peri_col[idx * BLOCK_SIZE + i]-=peri_col[idx * BLOCK_SIZE+ j]*dia[j * BLOCK_SIZE + i];
                    peri_col[idx * BLOCK_SIZE + i] /= dia[i * BLOCK_SIZE+ i];
                    }
                    }

                    if (tx < BLOCK_SIZE) { //peri-row
                    idx=tx;
                    array_offset = (offset+1)*matrix_dim+offset;
                    for(i=1; i < BLOCK_SIZE; i++){
                    m[array_offset+(bx+1)*BLOCK_SIZE+idx] = peri_row[i*BLOCK_SIZE+idx];
                    array_offset += matrix_dim;
                    }
                    } else { //peri-col
                    idx=tx - BLOCK_SIZE;
                    array_offset = (offset+(bx+1)*BLOCK_SIZE)*matrix_dim+offset;
                    for(i=0; i < BLOCK_SIZE; i++){
                    m[array_offset+idx] =  peri_col[i*BLOCK_SIZE+idx];
                    array_offset += matrix_dim;
                    }
                    }

                }
            }
        }
    }
}
extern "C"

void lud_internal (float *m, const size_t matrix_dim, const int offset) {
    #pragma HLS INTERFACE m_axi port=m offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=matrix_dim
    #pragma HLS INTERFACE s_axilite port=offset
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=peri_row complete dim=1
    #pragma HLS ARRAY_PARTITION variable=peri_col complete dim=1
    #pragma HLS ARRAY_PARTITION variable=peri_row complete dim=1
    #pragma HLS ARRAY_PARTITION variable=peri_col complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float peri_row [BLOCK_SIZE*BLOCK_SIZE];
                    float peri_col [BLOCK_SIZE*BLOCK_SIZE];
                    int  bx = _bid_x;
                    int  by = _bid_y;

                    int  tx = _tid_x;
                    int  ty = _tid_y;

                    float sum;

                    int global_row_id = offset + (by+1)*BLOCK_SIZE;
                    int global_col_id = offset + (bx+1)*BLOCK_SIZE;

                    peri_row[ty * BLOCK_SIZE + tx] = m[(offset+ty)*matrix_dim+global_col_id+tx];
                    peri_col[ty * BLOCK_SIZE + tx] = m[(global_row_id+ty)*matrix_dim+offset+tx];

                    int i;
                    sum = 0;
                    for (i=0; i < BLOCK_SIZE; i++)
                    sum += peri_col[ty * BLOCK_SIZE + i] * peri_row[i * BLOCK_SIZE + tx];

                    m[(global_row_id+ty)*matrix_dim+global_col_id+tx] -= sum;

                }
            }
        }
    }
}
