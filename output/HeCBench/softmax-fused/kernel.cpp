#include "kernel.h"

// --- from main.cu ---
__inline__ void copy_vector(Datatype *dst, const Datatype *src);

template <>
__inline__ void copy_vector<BFloat16, 1>(BFloat16 *dst,
                                                    const BFloat16 *src) {
  *dst = *src;
}

  inline T operator()(T a, T b) const { return a + b; }

  inline T operator()(T a, T b) const {
    return a < b ? b : a;
  }

inline void warp_reduce(acc_t *sum) {
  ReduceOp<acc_t> r;
#pragma unroll
  for (int offset = WARP_SIZE / 2; offset > 0; offset /= 2) {
#pragma unroll
    for (int i = 0; i < WARP_BATCH; ++i) {
      acc_t b = WARP_SHFL_XOR_NATIVE(sum[i], offset, WARP_SIZE);
      sum[i] = r(sum[i], b);
    }
  }
}
