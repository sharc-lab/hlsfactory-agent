#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
using std::min;
using std::max;

// CUDA thread/block dimension constants (adjust for your workload)
#ifndef BLOCK_DIM_X
#define BLOCK_DIM_X 256
#endif
#ifndef GRID_DIM_X
#define GRID_DIM_X 1
#endif

// --- from main.cu ---
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <type_traits>

#define CHECK_CUDA(func)                                                       \
{                                                                              \
    cudaError_t status = func;                                                 \
}

// Casts an fp16 input to the restricted values of float4_e2m1,
// that is to say [0., 0.5, 1.0, 1.5, 2.0, 3.0, 4.0, 6.0, -0.0, -0.5, -1.0, -1.5, -2.0, -3.0, -4.0, -6.0].
template<typename float_type, uint32_t half_exp_bits, uint32_t half_mantissa_bits, uint32_t half_exp_bias>

template<typename float_type, uint32_t half_exp_bits, uint32_t half_mantissa_bits, uint32_t half_exp_bias, uint16_t val_to_add, uint16_t sign_exponent_mask>

template <typename T>

template <typename T>



// --- from common.h ---
// Check for bfloat16 support
// V100 is compute capability 7.0 and doesn't support __nv_bfloat16 (requires
// >= 8.0) ROCm/HIP generally supports bfloat16 on modern GPUs

#include <math.h>
#include <cstdint>
#include <cstdio>
#include <stdexcept>

#define FLOAT16_MANTISSA_BITS 10
#define FLOAT16_EXP_BITS 5
#define FLOAT16_EXP_BIAS 15

#define FLOAT4_MANTISSA_BITS 1
#define FLOAT4_EXP_BITS 2
#define FLOAT4_EXP_BIAS 1

#define FLOAT8_E8M0_MAX_EXP 127

#define BFLOAT16_MANTISSA_BITS 7
#define BFLOAT16_EXP_BITS 8
#define BFLOAT16_EXP_BIAS 127

#define FLOAT16_VAL_TO_ADD \
  (1 << (FLOAT16_MANTISSA_BITS - FLOAT4_MANTISSA_BITS - 1))
#define FLOAT16_SIGN_EXPONENT_MASK \
  (((1 << (FLOAT16_EXP_BITS + 1)) - 1) << FLOAT16_MANTISSA_BITS)

#define BFLOAT16_VAL_TO_ADD \
  (1 << (BFLOAT16_MANTISSA_BITS - FLOAT4_MANTISSA_BITS - 1))
#define BFLOAT16_SIGN_EXPONENT_MASK \
  (((1 << (BFLOAT16_EXP_BITS + 1)) - 1) << BFLOAT16_MANTISSA_BITS)

template <typename T>
int bf16_or_half2int_rn(const T h);

template <typename T>
T float_to_bf16_or_half(const float x);

template <typename T>
float bf16_or_half_to_float(const T x);

template <typename T>
inline T shfl_xor_bf16_or_half(T x, int laneMask) {
  return 0;
}

// Definitions

template <>
inline int bf16_or_half2int_rn(const __half h) {
  return __half2int_rn(h);
}

template <>
inline int bf16_or_half2int_rn(const __nv_bfloat16 h) {
  return __bfloat162int_rn(h);
}

template <>
inline __half float_to_bf16_or_half(const float x) {
  return __float2half(x);
}

template <>
inline __nv_bfloat16 float_to_bf16_or_half(const float x) {
  return __float2bfloat16(x);
}

template <>
inline float bf16_or_half_to_float(const __half x) {
  return __half2float(x);
}

template <>
inline float bf16_or_half_to_float(const __nv_bfloat16 x) {
  return __bfloat162float(x);
}


// --- from verify.h ---
// basic check 
template <typename T>
void verify(T *src, T *dst, int numel) {
  bool ok = true;
  for (int i = 0; i < numel; i++) {
    float s = src[i], d = dst[i]; 
    if (s == 0.25) {
      if (d != 0.f) {
        printf("%f %f\n", s, d);
        ok = false;
      }
    }
    else if (s == 0.75f || s == 1.25f) {
      if (d != 1.f) {
        printf("%f %f\n", s, d);
        ok = false;
      }
    }
    else if (s == 1.75f || s == 2.5f) {
      if (d != 2.f) {
        printf("%f %f\n", s, d);
        ok = false;
      }
    }
    else if (s == 3.5f || s == 5.0f) {
      if (d != 4.f) {
        printf("%f %f\n", s, d);
        ok = false;
      }
    }
    else if (s > 0.5f && s < 0.75f) {
      if (d != 0.5f) {
        printf("%f %f\n", s, d);
        ok = false;
      }
    }
    else if (s > 0.f && s < 0.25f) {
      if (d != 0.f) {
        printf("%f %f\n", s, d);
        ok = false;
      }
    }
    else if (s > 0.25f && s < 0.5f) {
      if (d != 0.5f) {
        printf("%f %f\n", s, d);
        ok = false;
      }
    }
    else if (s > 0.75f && s < 1.f) {
      if (d != 1.f) {
        printf("%f %f\n", s, d);
        ok = false;
      }
    }
    else if (s < -0.5f && s > -0.75f) {
      if (d != -0.5f) {
        printf("%f %f\n", s, d);
        ok = false;
      }
    }
    else if (s < 0.f && s > -0.25f) {
      if (d != -0.f) {
        printf("%f %f\n", s, d);
        ok = false;
      }
    }
    else if (s < -0.25f && s > -0.5f) {
      if (d != -0.5f) {
        printf("%f %f\n", s, d);
        ok = false;
      }
    }
    else if (s < -0.75f && s > -1.f) {
      if (d != -1.f) {
        printf("%f %f\n", s, d);
        ok = false;
      }
    }
    else {
      if (fabsf(s - d) > 1e-3f) {
        printf("%f %f\n", s, d);
        ok = false;
      }
    }
    if (!ok) break;
  }
  printf("%s\n", ok ? "PASS" : "FAIL");
}
