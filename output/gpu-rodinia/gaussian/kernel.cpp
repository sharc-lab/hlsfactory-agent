#include "kernel.h"

// --- from gaussian.cu ---
 extern "C"
 ** Fan1() -- Calculate multiplier matrix
 ** Pay attention to the index.  Index i give the range
 ** which starts from 0 to range-1.  The real values of
 ** the index should be adjust and related with the value
 ** of t which is defined on the ForwardSub().
 **-------------------------------------------------------
 */
void Fan1(float *m_cuda, float *a_cuda, int Size, int t)
{
    #pragma HLS INTERFACE m_axi port=m offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=Size
    #pragma HLS INTERFACE s_axilite port=t
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE m_axi port=m offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=Size
    #pragma HLS INTERFACE s_axilite port=t
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    //if(_tid_x + _bid_x * BLOCK_DIM_X >= Size-1-t) printf(".");
                    //printf("blockIDx.x:%d,_tid_x:%d,Size:%d,t:%d,Size-1-t:%d\n",_bid_x,_tid_x,Size,t,Size-1-t);

                    if(_tid_x + _bid_x * BLOCK_DIM_X >= Size-1-t) return;
                    *(m_cuda+Size*(BLOCK_DIM_X*_bid_x+_tid_x+t+1)+t) = *(a_cuda+Size*(BLOCK_DIM_X*_bid_x+_tid_x+t+1)+t) / *(a_cuda+Size*t+t);

                }
            }
        }
    }
_x * BLOCK_DIM_X >= Size-1-t) printf(".");
                    //printf("blockIDx.x:%d,_tid_x:%d,Size:%d,t:%d,Size-1-t:%d\n",_bid_x,_tid_x,Size,t,Size-1-t);

                    if(_tid_x + _bid_x * BLOCK_DIM_X >= Size-1-t) return;
                    *(m_cuda+Size*(BLOCK_DIM_X*_bid_x+_tid_x+t+1)+t) = *(a_cuda+Size*(BLOCK_DIM_X*_bid_x+_tid_x+t+1)+t) / *(a_cuda+Size*t+t);

                }
            }
        }
    }
}
extern "C"

 ** Fan2() -- Modify the matrix A into LUD
 **-------------------------------------------------------
 */ 

void Fan2(float *m_cuda, float *a_cuda, float *b_cuda,int Size, int j1, int t)
{
    #pragma HLS INTERFACE m_axi port=m offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=Size
    #pragma HLS INTERFACE s_axilite port=j1
    #pragma HLS INTERFACE s_axilite port=t
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE m_axi port=m offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=Size
    #pragma HLS INTERFACE s_axilite port=j1
    #pragma HLS INTERFACE s_axilite port=t
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    if(_tid_x + _bid_x * BLOCK_DIM_X >= Size-1-t) return;
                    if(_tid_y + _bid_y * BLOCK_DIM_Y >= Size-t) return;

                    int xidx = _bid_x * BLOCK_DIM_X + _tid_x;
                    int yidx = _bid_y * BLOCK_DIM_Y + _tid_y;
                    //printf("_bid_x:%d,_tid_x:%d,_bid_y:%d,_tid_y:%d,BLOCK_DIM_X:%d,BLOCK_DIM_Y:%d\n",_bid_x,_tid_x,_bid_y,_tid_y,BLOCK_DIM_X,BLOCK_DIM_Y);

                    a_cuda[Size*(xidx+1+t)+(yidx+t)] -= m_cuda[Size*(xidx+1+t)+t] * a_cuda[Size*t+(yidx+t)];
                    //a_cuda[xidx+1+t][yidx+t] -= m_cuda[xidx+1+t][t] * a_cuda[t][yidx+t];
                    if(yidx == 0){
                    //printf("_bid_x:%d,_tid_x:%d,_bid_y:%d,_tid_y:%d,BLOCK_DIM_X:%d,BLOCK_DIM_Y:%d\n",_bid_x,_tid_x,_bid_y,_tid_y,BLOCK_DIM_X,BLOCK_DIM_Y);
                    //printf("xidx:%d,yidx:%d\n",xidx,yidx);
                    b_cuda[xidx+1+t] -= m_cuda[Size*(xidx+1+t)+(yidx+t)] * b_cuda[t];
                    }

                }
            }
        }
    }
+1+t)+(yidx+t)] -= m_cuda[Size*(xidx+1+t)+t] * a_cuda[Size*t+(yidx+t)];
                    //a_cuda[xidx+1+t][yidx+t] -= m_cuda[xidx+1+t][t] * a_cuda[t][yidx+t];
                    if(yidx == 0){
                    //printf("_bid_x:%d,_tid_x:%d,_bid_y:%d,_tid_y:%d,BLOCK_DIM_X:%d,BLOCK_DIM_Y:%d\n",_bid_x,_tid_x,_bid_y,_tid_y,BLOCK_DIM_X,BLOCK_DIM_Y);
                    //printf("xidx:%d,yidx:%d\n",xidx,yidx);
                    b_cuda[xidx+1+t] -= m_cuda[Size*(xidx+1+t)+(yidx+t)] * b_cuda[t];
                    }

                }
            }
        }
    }
}
