#include "kernel.h"

// --- from main.cu ---
extern "C"
void PTTWAC_soa_asta(const int A, 
                                const int B, 
                                const int b, 
                                  T * input, 
                                int * finished, 
                                int * head) 
{
    #pragma HLS INTERFACE s_axilite port=A
    #pragma HLS INTERFACE s_axilite port=B
    #pragma HLS INTERFACE s_axilite port=b
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=finished offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=head offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=lmem complete dim=1

    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
    #pragma HLS PIPELINE II=1

        int lmem[2];

        const int tid = _tid_x;
        int       m   = A * B - 1;

        if(tid == 0) // Dynamic fetch
        lmem[1] = (head[0] += 1);

        while(lmem[1] < m) {
        int next_in_cycle = (lmem[1] * A) - m * (lmem[1] / B);
        if(next_in_cycle == lmem[1]) {
        if(tid == 0) // Dynamic fetch
        lmem[1] = (head[0] += 1);
        continue;
        }
        T   data1, data2, data3, data4;
        int i = tid;
        if(i < b)
        data1 = input[lmem[1] * b + i];
        i += BLOCK_DIM_X;
        if(i < b)
        data2 = input[lmem[1] * b + i];
        i += BLOCK_DIM_X;
        if(i < b)
        data3 = input[lmem[1] * b + i];
        i += BLOCK_DIM_X;
        if(i < b)
        data4 = input[lmem[1] * b + i];

        if(tid == 0) {
        //make sure the read is not cached
        lmem[0] = atomicAdd(&finished[lmem[1]], 0);
        }

        for(; lmem[0] == 0; next_in_cycle = (next_in_cycle * A) - m * (next_in_cycle / B)) {
        T backup1, backup2, backup3, backup4;
        i = tid;
        if(i < b)
        backup1 = input[next_in_cycle * b + i];
        i += BLOCK_DIM_X;
        if(i < b)
        backup2 = input[next_in_cycle * b + i];
        i += BLOCK_DIM_X;
        if(i < b)
        backup3 = input[next_in_cycle * b + i];
        i += BLOCK_DIM_X;
        if(i < b)
        backup4 = input[next_in_cycle * b + i];

        if(tid == 0) {
        lmem[0] = (finished[next_in_cycle] = (int)1);
        }

        if(!lmem[0]) {
        i = tid;
        if(i < b)
        input[next_in_cycle * b + i] = data1;
        i += BLOCK_DIM_X;
        if(i < b)
        input[next_in_cycle * b + i] = data2;
        i += BLOCK_DIM_X;
        if(i < b)
        input[next_in_cycle * b + i] = data3;
        i += BLOCK_DIM_X;
        if(i < b)
        input[next_in_cycle * b + i] = data4;
        }
        i = tid;
        if(i < b)
        data1 = backup1;
        i += BLOCK_DIM_X;
        if(i < b)
        data2 = backup2;
        i += BLOCK_DIM_X;
        if(i < b)
        data3 = backup3;
        i += BLOCK_DIM_X;
        if(i < b)
        data4 = backup4;
        }

        if(tid == 0) // Dynamic fetch
        lmem[1] = (head[0] += 1);
        }

    }
}
