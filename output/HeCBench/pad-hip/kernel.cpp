#include "kernel.h"

// --- from kernel.cu ---
extern "C"
void Padding_kernel(int n, int m, int pad, int n_tasks, float alpha,
                    T * matrix_out,
                    const T *matrix,
                    int * flags
#ifdef DYNAMIC_PARTITION
                    , int * worklist
#endif
    ) {
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=m
    #pragma HLS INTERFACE s_axilite port=pad
    #pragma HLS INTERFACE s_axilite port=n_tasks
    #pragma HLS INTERFACE s_axilite port=alpha
    #pragma HLS INTERFACE m_axi port=matrix_out offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=matrix offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=DYNAMIC_PARTITION offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=#endif offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=l_mem complete dim=1

    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
    #pragma HLS PIPELINE II=1


        #ifdef DYNAMIC_PARTITION
        int l_mem[4096];
        int* l_tmp = l_mem;
        #endif

        #ifdef DYNAMIC_PARTITION
        Partitioner p = partitioner_create(n_tasks, alpha, worklist, l_tmp);
        #else
        Partitioner p = partitioner_create(n_tasks, alpha);
        #endif

        const int matrix_size = m * (n + pad);
        const int matrix_size_align =
        (matrix_size + BLOCK_DIM_X * REGS - 1) / (BLOCK_DIM_X * REGS) * (BLOCK_DIM_X * REGS);

        for(int my_s = gpu_first(&p); gpu_more(&p); my_s = gpu_next(&p)) {

        // Declare on-chip memory
        T   reg[REGS];
        int pos      = matrix_size_align - 1 - (my_s * REGS * BLOCK_DIM_X + _tid_x);
        int my_s_row = pos / (n + pad);
        int my_x     = pos % (n + pad);
        int pos2     = my_s_row * n + my_x;
        // Load in on-chip memory
        #pragma unroll
        for(int j = 0; j < REGS; j++) {
        if(pos2 >= 0 && my_x < n && pos2 < matrix_size)
        reg[j] = matrix[pos2];
        else
        reg[j] = 0;
        pos -= BLOCK_DIM_X;
        my_s_row = pos / (n + pad);
        my_x     = pos % (n + pad);
        pos2     = my_s_row * n + my_x;
        }

        // Set global synch
        if(_tid_x == 0) {
        #ifdef DYNAMIC_PARTITION
        while(atomicAdd_system(&flags[my_s], 0) == 0) {
        }
        atomicAdd_system(&flags[my_s + 1], 1);
        #else
        while((flags[my_s] += 0) == 0) {
        }
        (flags[my_s + 1] += 1);
        #endif
        }

        pos = matrix_size_align - 1 - (my_s * REGS * BLOCK_DIM_X + _tid_x);
        // Store to global memory
        #pragma unroll
        for(int j = 0; j < REGS; j++) {
        if(pos >= 0 && pos < matrix_size)
        matrix_out[pos] = reg[j];
        pos -= BLOCK_DIM_X;
        }
        }

    }
}
