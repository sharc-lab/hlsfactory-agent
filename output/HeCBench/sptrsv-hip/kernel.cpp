#include "kernel.h"

// --- from sptrsv_syncfree.cu ---
int atomic_load(const int *addr)
{
  const volatile int *vaddr = addr; // volatile to bypass cache
  //__threadfence(); // for seq_cst loads. Remove for acquire semantics.
  const int value = *vaddr;
  // fence to ensure that dependent reads are correctly ordered
  __threadfence(); 
  return value; 
}

void atomic_store(int *addr, int value)
{
  volatile int *vaddr = addr; // volatile to bypass cache
  // fence to ensure that previous non-atomic stores are visible to other threads
  __threadfence(); 
  *vaddr = value;
}
extern "C"

void sptrsv_mix(
    const int        * csrRowPtr,
    const int        * csrColIdx,
    const VALUE_TYPE * csrVal,
    int              * get_value,
    const int        m,
    const VALUE_TYPE * b,
    VALUE_TYPE       * x,
    const int        * warp_num,
    const int        Len)
{
    #pragma HLS INTERFACE m_axi port=csrRowPtr offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=csrColIdx offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=csrVal offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=get_value offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=m
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=warp_num offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=Len
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_left_sum complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int local_id = _tid_x;
            const int global_id = _bid_x * BLOCK_DIM_X + local_id;
            const int warp_id = global_id/WARP_SIZE;

            int row;
            VALUE_TYPE s_left_sum[WARP_PER_BLOCK*WARP_SIZE];

            if(warp_id>=(Len-1)) return;

            const int lane_id = (WARP_SIZE - 1) & local_id;

            if(warp_num[warp_id+1]>(warp_num[warp_id]+1))
            {
            //thread
            row =warp_num[warp_id]+lane_id;
            if(row>=m) return;

            int col,j,i;
            VALUE_TYPE xi;
            VALUE_TYPE left_sum=0;
            i=row;
            j=csrRowPtr[i];

            while(j<csrRowPtr[i+1])
            {
            col=csrColIdx[j];
            if(atomic_load(&get_value[col])==1)
            {
            left_sum+=csrVal[j]*x[col];
            j++;
            col=csrColIdx[j];
            }
            if(i==col)
            {
            xi = (b[i] - left_sum) / csrVal[csrRowPtr[i+1]-1];
            x[i] = xi;
            atomic_store(&get_value[i], 1);
            j++;
            }
            }
            }
            else
            {
            row = warp_num[warp_id];
            if(row>=m)
            return;

            int col,j=csrRowPtr[row]  + lane_id;
            VALUE_TYPE xi,sum=0;
            while(j < (csrRowPtr[row+1]-1))
            {
            col=csrColIdx[j];
            if(atomic_load(&get_value[col])==1)
            {
            sum += x[col] * csrVal[j];
            j += WARP_SIZE;
            }
            }

            s_left_sum[local_id]=sum;

            for (int offset = WARP_SIZE/2; offset > 0; offset /= 2)
            {
            if(lane_id < offset)
            {
            s_left_sum[local_id] += s_left_sum[local_id+offset];
            }
            }

            if (!lane_id)
            {
            xi = (b[row] - s_left_sum[local_id]) / csrVal[csrRowPtr[row+1]-1];
            x[row]=xi;
            atomic_store(&get_value[row], 1);
            }
            }

        }
    }
}
