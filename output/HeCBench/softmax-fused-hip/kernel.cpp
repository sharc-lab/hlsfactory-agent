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
