#include "kernel.h"

// --- from main.cu ---
inline uint lsr(uint x, int sa) {
  if(sa > 0 && sa < 32) return (x >> sa);
  return x;
}

inline uint lsl(uint x, int sa) {
  if (sa > 0 && sa < 32) return (x << sa);
  return x;
}

inline uint set_bit(uint &data, int y) {
  data |= lsl(1, y);
  return data;
}
