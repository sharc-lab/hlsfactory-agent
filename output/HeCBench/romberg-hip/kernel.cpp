#include "kernel.h"

// --- from main.cu ---
inline double f(double x)
{
  return exp(x)*sin(x);
}

inline unsigned int getFirstSetBitPos(int n)
{
  return log2((float)(n&-n))+1;
}
extern "C"

void romberg(double a, double b, double *result)  
{
    #pragma HLS INTERFACE s_axilite port=a
    #pragma HLS INTERFACE s_axilite port=b
    #pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=smem complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            double smem[4096];
            double diff = (b-a)/GRID_DIM_X, step;
            int k;
            int max_eval = (1<<(ROW_SIZE-1));
            b = a + (_bid_x+1)*diff;
            a += _bid_x*diff;

            step = (b-a)/max_eval;

            double local_col[ROW_SIZE];  // specific to the row size
            for(int i = 0; i < ROW_SIZE; i++) local_col[i] = 0.0;
            if(!_tid_x)
            {
            k = BLOCK_DIM_X;
            local_col[0] = f(a) + f(b);
            }
            else
            k = _tid_x;

            for(; k < max_eval; k += BLOCK_DIM_X)
            local_col[ROW_SIZE - getFirstSetBitPos(k)] += 2.0*f(a + step*k);

            for(int i = 0; i < ROW_SIZE; i++)
            smem[ROW_SIZE*_tid_x + i] = local_col[i];

            if(_tid_x < ROW_SIZE)
            {
            double sum = 0.0;
            for(int i = _tid_x; i < BLOCK_DIM_X*ROW_SIZE; i+=ROW_SIZE)
            sum += smem[i];
            smem[_tid_x] = sum;
            }

            if(!_tid_x)
            {
            double *table = local_col;
            table[0] = smem[0];

            for(int k = 1; k < ROW_SIZE; k++)
            table[k] = table[k-1] + smem[k];

            for(int k = 0; k < ROW_SIZE; k++)
            table[k]*= (b-a)/(1<<(k+1));

            for(int col = 0 ; col < ROW_SIZE-1 ; col++)
            for(int row = ROW_SIZE-1; row > col; row--)
            table[row] = table[row] + (table[row] - table[row-1])/((1<<(2*col+1))-1);

            result[_bid_x] = table[ROW_SIZE-1];
            }

        }
    }
}
