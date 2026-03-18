#include "kernel.h"

// --- from main.cu ---
half2 half_max(const half2 a, const half2 b) {
  const half2 sub = __hsub2(a, b);
  const unsigned sign = (*reinterpret_cast<const unsigned*>(&sub)) & 0x80008000u;
  const unsigned sw = 0x00003210 | (((sign >> 21) | (sign >> 13)) * 0x11);
  const unsigned int res = __byte_perm(*reinterpret_cast<const unsigned*>(&a), 
                                       *reinterpret_cast<const unsigned*>(&b), sw);
  return *reinterpret_cast<const half2*>(&res);
}

half half_max(const half a, const half b) {
  const half sub = __hsub(a, b);
  const unsigned sign = (*reinterpret_cast<const short*>(&sub)) & 0x8000u;
  const unsigned sw = 0x00000010 | ((sign >> 13) * 0x11);
  const unsigned short res = __byte_perm(*reinterpret_cast<const short*>(&a), 
                                         *reinterpret_cast<const short*>(&b), sw);
  return *reinterpret_cast<const half*>(&res);
}
extern "C"

void hmax(T const * const a,
          T const * const b,
          T * const r,
          const size_t size)
{
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=r offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            for (size_t i = _tid_x + BLOCK_DIM_X * _bid_x;
            i < size; i += BLOCK_DIM_X * GRID_DIM_X)
            r[i] = half_max(a[i], b[i]);

        }
    }
}
