#include "kernel.h"

// --- from needle_kernel.cu ---
int maximum( int a,
		 int b,
		 int c){

int k;
if( a <= b )
k = b;
else 
k = a;

if( k <=c )
return(c);
else
return(k);

}
extern "C"

void needle_cuda_shared_1(  int* referrence,
			  int* matrix_cuda, 
			  int cols,
			  int penalty,
			  int i,
			  int block_width) 
{
    #pragma HLS INTERFACE m_axi port=referrence offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=matrix_cuda offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=cols
    #pragma HLS INTERFACE s_axilite port=penalty
    #pragma HLS INTERFACE s_axilite port=i
    #pragma HLS INTERFACE s_axilite port=block_width
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=temp complete dim=1
    #pragma HLS ARRAY_PARTITION variable=ref complete dim=1
    #pragma HLS ARRAY_PARTITION variable=temp complete dim=1
    #pragma HLS ARRAY_PARTITION variable=ref complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int bx = _bid_x;
            int tx = _tid_x;

            int b_index_x = bx;
            int b_index_y = i - 1 - bx;

            int index   = cols * BLOCK_SIZE * b_index_y + BLOCK_SIZE * b_index_x + tx + ( cols + 1 );
            int index_n   = cols * BLOCK_SIZE * b_index_y + BLOCK_SIZE * b_index_x + tx + ( 1 );
            int index_w   = cols * BLOCK_SIZE * b_index_y + BLOCK_SIZE * b_index_x + ( cols );
            int index_nw =  cols * BLOCK_SIZE * b_index_y + BLOCK_SIZE * b_index_x;

            int temp[BLOCK_SIZE+1][BLOCK_SIZE+1];
            int ref[BLOCK_SIZE][BLOCK_SIZE];

            if (tx == 0)
            temp[tx][0] = matrix_cuda[index_nw];

            for ( int ty = 0 ; ty < BLOCK_SIZE ; ty++)
            ref[ty][tx] = referrence[index + cols * ty];

            temp[tx + 1][0] = matrix_cuda[index_w + cols * tx];

            temp[0][tx + 1] = matrix_cuda[index_n];


            for( int m = 0 ; m < BLOCK_SIZE ; m++){

            if ( tx <= m ){

            int t_index_x =  tx + 1;
            int t_index_y =  m - tx + 1;

            temp[t_index_y][t_index_x] = maximum( temp[t_index_y-1][t_index_x-1] + ref[t_index_y-1][t_index_x-1],
            temp[t_index_y][t_index_x-1]  - penalty,
            temp[t_index_y-1][t_index_x]  - penalty);



            }

            }

            for( int m = BLOCK_SIZE - 2 ; m >=0 ; m--){

            if ( tx <= m){

            int t_index_x =  tx + BLOCK_SIZE - m ;
            int t_index_y =  BLOCK_SIZE - tx;

            temp[t_index_y][t_index_x] = maximum( temp[t_index_y-1][t_index_x-1] + ref[t_index_y-1][t_index_x-1],
            temp[t_index_y][t_index_x-1]  - penalty,
            temp[t_index_y-1][t_index_x]  - penalty);

            }
            }

            for ( int ty = 0 ; ty < BLOCK_SIZE ; ty++)
            matrix_cuda[index + ty * cols] = temp[ty+1][tx+1];


        }
    }
}
extern "C"

void needle_cuda_shared_2(  int* referrence,
			  int* matrix_cuda, 
			 
			  int cols,
			  int penalty,
			  int i,
			  int block_width) 
{
    #pragma HLS INTERFACE m_axi port=referrence offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=matrix_cuda offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=cols
    #pragma HLS INTERFACE s_axilite port=penalty
    #pragma HLS INTERFACE s_axilite port=i
    #pragma HLS INTERFACE s_axilite port=block_width
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=temp complete dim=1
    #pragma HLS ARRAY_PARTITION variable=ref complete dim=1
    #pragma HLS ARRAY_PARTITION variable=temp complete dim=1
    #pragma HLS ARRAY_PARTITION variable=ref complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            int bx = _bid_x;
            int tx = _tid_x;

            int b_index_x = bx + block_width - i  ;
            int b_index_y = block_width - bx -1;

            int index   = cols * BLOCK_SIZE * b_index_y + BLOCK_SIZE * b_index_x + tx + ( cols + 1 );
            int index_n   = cols * BLOCK_SIZE * b_index_y + BLOCK_SIZE * b_index_x + tx + ( 1 );
            int index_w   = cols * BLOCK_SIZE * b_index_y + BLOCK_SIZE * b_index_x + ( cols );
            int index_nw =  cols * BLOCK_SIZE * b_index_y + BLOCK_SIZE * b_index_x;

            int temp[BLOCK_SIZE+1][BLOCK_SIZE+1];
            int ref[BLOCK_SIZE][BLOCK_SIZE];

            for ( int ty = 0 ; ty < BLOCK_SIZE ; ty++)
            ref[ty][tx] = referrence[index + cols * ty];

            if (tx == 0)
            temp[tx][0] = matrix_cuda[index_nw];


            temp[tx + 1][0] = matrix_cuda[index_w + cols * tx];

            temp[0][tx + 1] = matrix_cuda[index_n];


            for( int m = 0 ; m < BLOCK_SIZE ; m++){

            if ( tx <= m ){

            int t_index_x =  tx + 1;
            int t_index_y =  m - tx + 1;

            temp[t_index_y][t_index_x] = maximum( temp[t_index_y-1][t_index_x-1] + ref[t_index_y-1][t_index_x-1],
            temp[t_index_y][t_index_x-1]  - penalty,
            temp[t_index_y-1][t_index_x]  - penalty);

            }

            }

            for( int m = BLOCK_SIZE - 2 ; m >=0 ; m--){

            if ( tx <= m){

            int t_index_x =  tx + BLOCK_SIZE - m ;
            int t_index_y =  BLOCK_SIZE - tx;

            temp[t_index_y][t_index_x] = maximum( temp[t_index_y-1][t_index_x-1] + ref[t_index_y-1][t_index_x-1],
            temp[t_index_y][t_index_x-1]  - penalty,
            temp[t_index_y-1][t_index_x]  - penalty);

            }
            }

            for ( int ty = 0 ; ty < BLOCK_SIZE ; ty++)
            matrix_cuda[index + ty * cols] = temp[ty+1][tx+1];


        }
    }
}
