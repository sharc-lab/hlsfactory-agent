#include "kernel.h"

// --- from kernel.cu ---
extern "C"
void TaskQueue_gpu(const task_t * queue,
                   int * data,
                   int * consumed,
                   const int iterations,
                   const int offset,
                   const int gpuQueueSize)
{
    #pragma HLS INTERFACE m_axi port=queue offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=consumed offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=iterations
    #pragma HLS INTERFACE s_axilite port=offset
    #pragma HLS INTERFACE s_axilite port=gpuQueueSize
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=l_mem complete dim=1

    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
    #pragma HLS PIPELINE II=1

        int l_mem[4096];
        int& next = l_mem[0];
        task_t* t = (task_t*)&l_mem[1];

        const int tid       = _tid_x;
        const int tile_size = BLOCK_DIM_X;

        // Fetch task
        if(tid == 0) {
        next = (*consumed += 1);
        t->id = queue[next].id;
        t->op = queue[next].op;
        }
        while(next < gpuQueueSize) {
        // Compute task
        if(t->op == SIGNAL_WORK_KERNEL) {
        for(int i = 0; i < iterations; i++) {
        data[(t->id - offset) * tile_size + tid] += tile_size;
        }

        data[(t->id - offset) * tile_size + tid] += t->id;
        }
        if(t->op == SIGNAL_NOTWORK_KERNEL) {
        for(int i = 0; i < 1; i++) {
        data[(t->id - offset) * tile_size + tid] += tile_size;
        }

        data[(t->id - offset) * tile_size + tid] += t->id;
        }
        if(tid == 0) {
        next = (*consumed += 1);
        // Fetch task
        t->id = queue[next].id;
        t->op = queue[next].op;
        }
        }

    }
}
