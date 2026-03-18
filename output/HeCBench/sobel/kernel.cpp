#include "kernel.h"

// --- from kernels.cu ---
inline uchar4 convert_uchar4(float4 v) {
  uchar4 res;
  res.x = (uchar) ((v.x > 255.f) ? 255.f : (v.x < 0.f ? 0.f : v.x));
  res.y = (uchar) ((v.y > 255.f) ? 255.f : (v.y < 0.f ? 0.f : v.y));
  res.z = (uchar) ((v.z > 255.f) ? 255.f : (v.z < 0.f ? 0.f : v.z));
  res.w = (uchar) ((v.w > 255.f) ? 255.f : (v.w < 0.f ? 0.f : v.w));
  return res;
}
