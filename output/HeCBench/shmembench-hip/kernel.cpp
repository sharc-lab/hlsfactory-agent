#include "kernel.h"

// --- from shmem_kernels.cu ---
void shmem_swap(float4 *v1, float4 *v2){
  float4 tmp;
  tmp = *v2;
  *v2 = *v1;
  *v1 = tmp;
}

float4 init_val(int i){
  return float4(i, i+11, i+19, i+23);
}

float4 reduce_vector(float4 v1, float4 v2, float4 v3, float4 v4, float4 v5, float4 v6){
  return (v1 + v2 + v3 + v4 + v5 + v6);
}

void set_vector(float4 *target, int offset, float4 v){
  target[offset] = v;
}
extern "C"

void benchmark_shmem(float4 *g_data){
    #pragma HLS INTERFACE m_axi port=g_data offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shm_buffer complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            float4 shm_buffer[BLOCK_SIZE*6];

            int tid = _tid_x;
            int globaltid = _bid_x*BLOCK_DIM_X + tid;
            set_vector(shm_buffer, tid+0*BLOCK_DIM_X, init_val(tid));
            set_vector(shm_buffer, tid+1*BLOCK_DIM_X, init_val(tid+1));
            set_vector(shm_buffer, tid+2*BLOCK_DIM_X, init_val(tid+3));
            set_vector(shm_buffer, tid+3*BLOCK_DIM_X, init_val(tid+7));
            set_vector(shm_buffer, tid+4*BLOCK_DIM_X, init_val(tid+13));
            set_vector(shm_buffer, tid+5*BLOCK_DIM_X, init_val(tid+17));

            #pragma unroll 32
            for(int j=0; j<TOTAL_ITERATIONS; j++){
            shmem_swap(shm_buffer+tid+0*BLOCK_DIM_X, shm_buffer+tid+1*BLOCK_DIM_X);
            shmem_swap(shm_buffer+tid+2*BLOCK_DIM_X, shm_buffer+tid+3*BLOCK_DIM_X);
            shmem_swap(shm_buffer+tid+4*BLOCK_DIM_X, shm_buffer+tid+5*BLOCK_DIM_X);

            shmem_swap(shm_buffer+tid+1*BLOCK_DIM_X, shm_buffer+tid+2*BLOCK_DIM_X);
            shmem_swap(shm_buffer+tid+3*BLOCK_DIM_X, shm_buffer+tid+4*BLOCK_DIM_X);
            }

            g_data[globaltid] = reduce_vector(shm_buffer[tid+0*BLOCK_DIM_X],
            shm_buffer[tid+1*BLOCK_DIM_X],
            shm_buffer[tid+2*BLOCK_DIM_X],
            shm_buffer[tid+3*BLOCK_DIM_X],
            shm_buffer[tid+4*BLOCK_DIM_X],
            shm_buffer[tid+5*BLOCK_DIM_X]);

        }
    }
}
