#include "kernel.h"

// --- from kernel.cu ---
float4 convert_float4(uchar4 v) {
  float4 res;
  res.x = (float) v.x;
  res.y = (float) v.y;
  res.z = (float) v.z;
  res.w = (float) v.w;
  return res;
}

uchar4 convert_uchar4_sat(float4 v) {
  uchar4 res;
  res.x = (unsigned char) ((v.x > 255.f) ? 255.f : (v.x < 0.f ? 0.f : v.x));
  res.y = (unsigned char) ((v.y > 255.f) ? 255.f : (v.y < 0.f ? 0.f : v.y));
  res.z = (unsigned char) ((v.z > 255.f) ? 255.f : (v.z < 0.f ? 0.f : v.z));
  res.w = (unsigned char) ((v.w > 255.f) ? 255.f : (v.w < 0.f ? 0.f : v.w));
  return res;
}
