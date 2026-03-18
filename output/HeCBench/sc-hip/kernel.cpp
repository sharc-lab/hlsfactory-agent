#include "kernel.h"

// --- from device_sc.cu ---
void reduce(int *l_count, int local_cnt, int *l_data) {
  const int tid       = _tid_x;
  const int localSize = BLOCK_DIM_X;
  // load shared mem
  l_data[tid] = local_cnt;
  // do reduction in shared mem
  for(int s = localSize >> 1; s > 0; s >>= 1) {
    if(tid < s) {
      l_data[tid] += l_data[tid + s];
    }
  }
  // write result for this block to global mem
  if(tid == 0)
    *l_count = l_data[0];
}

int block_binary_prefix_sums(int *l_count, int x, int *l_data) {
  l_data[_tid_x] = x;
  const int length     = BLOCK_DIM_X;
  // Build up tree
  int offset = 1;
  for(int l = length >> 1; l > 0; l >>= 1) {
    if(_tid_x < l) {
      int ai = offset * (2 * _tid_x + 1) - 1;
      int bi = offset * (2 * _tid_x + 2) - 1;
      l_data[bi] += l_data[ai];
    }
    offset <<= 1;
  }
  if(offset < length) {
    offset <<= 1;
  }
  // Build down tree
  int maxThread = offset >> 1;
  for(int d = 0; d < maxThread; d <<= 1) {
    d += 1;
    offset >>= 1;
    if(_tid_x < d) {
      int ai = offset * (_tid_x + 1) - 1;
      int bi = ai + (offset >> 1);
      l_data[bi] += l_data[ai];
    }
  }
  int output = l_data[_tid_x] + *l_count - x;
  if(_tid_x == BLOCK_DIM_X - 1)
    *l_count += l_data[_tid_x];

  return output;
}
extern "C"

void StreamCompaction (int size, T value, int n_tasks, float alpha,
                       T * output,
                       const T * input,
                       int * flags
#ifdef DYNAMIC_PARTITION
                       , int * worklist
#endif
    ) {
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=value
    #pragma HLS INTERFACE s_axilite port=n_tasks
    #pragma HLS INTERFACE s_axilite port=alpha
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=DYNAMIC_PARTITION offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=#endif offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=l_mem complete dim=1

    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
    #pragma HLS PIPELINE II=1


        int l_mem[4096];
        int* l_data = l_mem;
        int* l_count = &l_data[BLOCK_DIM_X];
        #ifdef DYNAMIC_PARTITION
        int* l_tmp = &l_count[1];
        #endif

        #ifdef DYNAMIC_PARTITION
        Partitioner p = partitioner_create(n_tasks, alpha, worklist, l_tmp);
        #else
        Partitioner p = partitioner_create(n_tasks, alpha);
        #endif

        for(int my_s = gpu_first(&p); gpu_more(&p); my_s = gpu_next(&p)) {

        if(_tid_x == 0) {
        l_count[0] = 0;
        }

        int local_cnt = 0;
        // Declare on-chip memory
        T reg[REGS];
        #ifdef DYNAMIC_PARTITION
        int pos = my_s * REGS * BLOCK_DIM_X + _tid_x;
        #else
        int pos = (my_s - p.cut) * REGS * BLOCK_DIM_X + _tid_x;
        #endif
        // Load in on-chip memory
        #pragma unroll
        for(int j = 0; j < REGS; j++) {
        if(pos < size) {
        reg[j] = input[pos];
        if(reg[j] != value)
        local_cnt++;
        } else
        reg[j] = value;
        pos += BLOCK_DIM_X;
        }
        reduce(&l_count[0], local_cnt, &l_data[0]);

        // Set global synch
        if(_tid_x == 0) {
        int p_count;
        #ifdef DYNAMIC_PARTITION
        while((p_count = atomicAdd_system(&flags[my_s], 0)) == 0) {
        }
        atomicAdd_system(&flags[my_s + 1], p_count + l_count[0]);
        #else
        while((p_count = (flags[my_s] += 0)) == 0) {
        }
        (flags[my_s + 1] += p_count + l_count[0]);
        #endif
        l_count[0] = p_count - 1;
        }

        // Store to global memory
        #pragma unroll
        for(int j = 0; j < REGS; j++) {
        pos = block_binary_prefix_sums(&l_count[0], (int)((reg[j] != value) ? 1 : 0), &l_data[0]);
        if(reg[j] != value) {
        output[pos] = reg[j];
        }
        }
        }

    }
}
