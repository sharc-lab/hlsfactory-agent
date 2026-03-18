#include "kernel.h"

// --- from backprop_cuda_kernel.cu ---
extern "C"
void bpnn_layerforward_CUDA(float *input_cuda,
	                   float *output_hidden_cuda,
					   float *input_hidden_cuda,
					   float *hidden_partial_sum,
					   int in,
					   int hid) 
{
    #pragma HLS INTERFACE m_axi port=input_cuda offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=output_hidden_cuda offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=input_hidden_cuda offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=hidden_partial_sum offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=in
    #pragma HLS INTERFACE s_axilite port=hid
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=input_node complete dim=1
    #pragma HLS ARRAY_PARTITION variable=weight_matrix complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                int by = _bid_y;
                int tx = _tid_x;
                int ty = _tid_y;

                int index =  ( hid + 1 ) * HEIGHT * by + ( hid + 1 ) * ty + tx + 1 + ( hid + 1 ) ;

                int index_in = HEIGHT * by + ty + 1;

                float input_node[HEIGHT];
                float weight_matrix[HEIGHT][WIDTH];

                if ( tx == 0 )
                input_node[ty] = input_cuda[index_in] ;

                weight_matrix[ty][tx] = input_hidden_cuda[index];

                weight_matrix[ty][tx] = weight_matrix[ty][tx] * input_node[ty];

                for ( int i = 1 ; i <= __log2f(HEIGHT) ; i++){

                int power_two = __powf(2, i);

                if( ty % power_two == 0 )
                weight_matrix[ty][tx] = weight_matrix[ty][tx] + weight_matrix[ty + power_two/2][tx];

                }

                //

                input_hidden_cuda[index] = weight_matrix[ty][tx];

                /*
                for ( unsigned int i = 2 ; i <= HEIGHT ; i *= 2){

                unsigned int power_two = i - 1;

                if( (ty & power_two) == 0 ) {
                weight_matrix[ty][tx] = weight_matrix[ty][tx] + weight_matrix[ty + power_two/2][tx];
                }

                }
                */

                if ( tx == 0 ) {
                hidden_partial_sum[by * hid + ty] = weight_matrix[tx][ty];
                }


            }
        }
    }
}
extern "C"

void bpnn_adjust_weights_cuda(float * delta,   
										 int hid,         
										 float * ly,      
										 int in,          
										 float * w,       
										 float * oldw)  									
{
    #pragma HLS INTERFACE m_axi port=delta offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=hid
    #pragma HLS INTERFACE m_axi port=ly offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=in
    #pragma HLS INTERFACE m_axi port=w offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=oldw offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1



                int by = _bid_y;

                int tx = _tid_x;
                int ty = _tid_y;

                int index =  ( hid + 1 ) * HEIGHT * by + ( hid + 1 ) * ty + tx + 1 + ( hid + 1 ) ;
                int index_y = HEIGHT * by + ty + 1;
                int index_x = tx + 1;
                //eta = 0.3;
                //momentum = 0.3;

                w[index] += ((ETA * delta[index_x] * ly[index_y]) + (MOMENTUM * oldw[index]));
                oldw[index] = ((ETA * delta[index_x] * ly[index_y]) + (MOMENTUM * oldw[index]));

                if (ty == 0 && by ==0){
                w[index_x] += ((ETA * delta[index_x]) + (MOMENTUM * oldw[index_x]));
                oldw[index_x] = ((ETA * delta[index_x]) + (MOMENTUM * oldw[index_x]));
                }


            }
        }
    }
}
