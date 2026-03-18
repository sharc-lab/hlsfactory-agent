#include "kernel.h"

// --- from hotspot.cu ---
extern "C"
void calculate_temp(int iteration,  //number of iteration
                               float *power,   //power input
                               float *temp_src,    //temperature input/output
                               float *temp_dst,    //temperature input/output
                               int grid_cols,  //Col of grid
                               int grid_rows,  //Row of grid
							   int border_cols,  // border offset 
							   int border_rows,  // border offset
                               float Cap,      //Capacitance
                               float Rx, 
                               float Ry, 
                               float Rz, 
                               float step, 
                               float time_elapsed){
    #pragma HLS INTERFACE s_axilite port=iteration
    #pragma HLS INTERFACE m_axi port=power offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=temp_src offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=temp_dst offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=grid_cols
    #pragma HLS INTERFACE s_axilite port=grid_rows
    #pragma HLS INTERFACE s_axilite port=border_cols
    #pragma HLS INTERFACE s_axilite port=border_rows
    #pragma HLS INTERFACE s_axilite port=Cap
    #pragma HLS INTERFACE s_axilite port=Rx
    #pragma HLS INTERFACE s_axilite port=Ry
    #pragma HLS INTERFACE s_axilite port=Rz
    #pragma HLS INTERFACE s_axilite port=step
    #pragma HLS INTERFACE s_axilite port=time_elapsed
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=temp_on_cuda complete dim=1
    #pragma HLS ARRAY_PARTITION variable=power_on_cuda complete dim=1
    #pragma HLS ARRAY_PARTITION variable=temp_t complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1


                    float temp_on_cuda[BLOCK_SIZE][BLOCK_SIZE];
                    float power_on_cuda[BLOCK_SIZE][BLOCK_SIZE];
                    float temp_t[BLOCK_SIZE][BLOCK_SIZE]; // saving temparary temperature result

                    float amb_temp = 80.0;
                    float step_div_Cap;
                    float Rx_1,Ry_1,Rz_1;

                    int bx = _bid_x;
                    int by = _bid_y;

                    int tx=_tid_x;
                    int ty=_tid_y;

                    step_div_Cap=step/Cap;

                    Rx_1=1/Rx;
                    Ry_1=1/Ry;
                    Rz_1=1/Rz;

                    // each block finally computes result for a small block
                    // after N iterations.
                    // it is the non-overlapping small blocks that cover
                    // all the input data

                    // calculate the small block size
                    int small_block_rows = BLOCK_SIZE-iteration*2;//EXPAND_RATE
                    int small_block_cols = BLOCK_SIZE-iteration*2;//EXPAND_RATE

                    // calculate the boundary for the block according to
                    // the boundary of its small block
                    int blkY = small_block_rows*by-border_rows;
                    int blkX = small_block_cols*bx-border_cols;
                    int blkYmax = blkY+BLOCK_SIZE-1;
                    int blkXmax = blkX+BLOCK_SIZE-1;

                    // calculate the global thread coordination
                    int yidx = blkY+ty;
                    int xidx = blkX+tx;

                    // load data if it is within the valid input range
                    int loadYidx=yidx, loadXidx=xidx;
                    int index = grid_cols*loadYidx+loadXidx;

                    if(IN_RANGE(loadYidx, 0, grid_rows-1) && IN_RANGE(loadXidx, 0, grid_cols-1)){
                    temp_on_cuda[ty][tx] = temp_src[index];  // Load the temperature data from global memory to shared memory
                    power_on_cuda[ty][tx] = power[index];// Load the power data from global memory to shared memory
                    }

                    // effective range within this block that falls within
                    // the valid range of the input data
                    // used to rule out computation outside the boundary.
                    int validYmin = (blkY < 0) ? -blkY : 0;
                    int validYmax = (blkYmax > grid_rows-1) ? BLOCK_SIZE-1-(blkYmax-grid_rows+1) : BLOCK_SIZE-1;
                    int validXmin = (blkX < 0) ? -blkX : 0;
                    int validXmax = (blkXmax > grid_cols-1) ? BLOCK_SIZE-1-(blkXmax-grid_cols+1) : BLOCK_SIZE-1;

                    int N = ty-1;
                    int S = ty+1;
                    int W = tx-1;
                    int E = tx+1;

                    N = (N < validYmin) ? validYmin : N;
                    S = (S > validYmax) ? validYmax : S;
                    W = (W < validXmin) ? validXmin : W;
                    E = (E > validXmax) ? validXmax : E;

                    bool computed;
                    for (int i=0; i<iteration ; i++){
                    computed = false;
                    if( IN_RANGE(tx, i+1, BLOCK_SIZE-i-2) &&  \
                    IN_RANGE(ty, i+1, BLOCK_SIZE-i-2) &&  \
                    IN_RANGE(tx, validXmin, validXmax) && \
                    IN_RANGE(ty, validYmin, validYmax) ) {
                    computed = true;
                    temp_t[ty][tx] =   temp_on_cuda[ty][tx] + step_div_Cap * (power_on_cuda[ty][tx] +
                    (temp_on_cuda[S][tx] + temp_on_cuda[N][tx] - 2.0*temp_on_cuda[ty][tx]) * Ry_1 +
                    (temp_on_cuda[ty][E] + temp_on_cuda[ty][W] - 2.0*temp_on_cuda[ty][tx]) * Rx_1 +
                    (amb_temp - temp_on_cuda[ty][tx]) * Rz_1);

                    }
                    if(i==iteration-1)
                    break;
                    if(computed)	 //Assign the computation range
                    temp_on_cuda[ty][tx]= temp_t[ty][tx];
                    }

                    // update the global memory
                    // after the last iteration, only threads coordinated within the
                    // small block perform the calculation and switch on ``computed''
                    if (computed){
                    temp_dst[index]= temp_t[ty][tx];
                    }

                }
            }
        }
    }
}
