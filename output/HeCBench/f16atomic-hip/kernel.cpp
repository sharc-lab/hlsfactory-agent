#include "kernel.h"

// --- from main.cu ---
extern "C"
void f16AtomicOnGlobalMem(__half* result, int n)
{
    #pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int tid = _bid_x * BLOCK_DIM_X + _tid_x;
            if (tid >= n) return;
            __half2 *result_v = reinterpret_cast<__half2*>(result);
            __half2 val {ZERO_FP16, ONE_FP16};
            unsafeAtomicAdd(&result_v[tid % BLOCK_SIZE], val);

        }
    }
      if (tid >= n) return;
            __half2 *result_v = reinterpret_cast<__half2*>(result);
            __half2 val {ZERO_FP16, ONE_FP16};
            unsafeAtomicAdd(&result_v[tid % BLOCK_SIZE], val);

        }
    }
}

void f16AtomicOnGlobalMem(__hip_bfloat16* result, int n)
{
  int tid = _bid_x * BLOCK_DIM_X + _tid_x;
  if (tid >= n) return;
  __hip_bfloat162 *result_v = reinterpret_cast<__hip_bfloat162*>(result);
  __hip_bfloat162 val {ZERO_BF16, ONE_BF16};
  unsafeAtomicAdd(&result_v[tid % BLOCK_SIZE], val);
}
