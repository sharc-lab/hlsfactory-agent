#include "kernel.h"

// --- from main.cu ---
extern "C"
void sampleKernel (Type *std, IdxType D, IdxType N) {
    #pragma HLS INTERFACE m_axi port=std offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=D
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                IdxType i = BLOCK_DIM_X * _bid_x + _tid_x;
                if (i < D) std[i] = sqrtf(std[i] / N);

            }
        }
    }
}
extern "C"

void sopKernel(
        Type * std, 
  const Type * data, 
  IdxType D, 
  IdxType N) 
{
    #pragma HLS INTERFACE m_axi port=std offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=D
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sstd complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                const int RowsPerBlkPerIter = TPB / ColsPerBlk;
                IdxType thisColId = _tid_x % ColsPerBlk;
                IdxType thisRowId = _tid_x / ColsPerBlk;
                IdxType colId = thisColId + ((IdxType)_bid_y * ColsPerBlk);
                IdxType rowId = thisRowId + ((IdxType)_bid_x * RowsPerBlkPerIter);
                Type thread_data = Type(0);
                const IdxType stride = RowsPerBlkPerIter * GRID_DIM_X;
                for (IdxType i = rowId; i < N; i += stride) {
                Type val = (colId < D) ? data[i * D + colId] : Type(0);
                thread_data += val * val;
                }
                Type sstd[ColsPerBlk];
                if (_tid_x < ColsPerBlk) sstd[_tid_x] = Type(0);

                atomicAdd(sstd + thisColId, thread_data);

                if (_tid_x < ColsPerBlk) atomicAdd(std + colId, sstd[thisColId]);

            }
        }
    }
}
