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
/*
 * approximate atan2 evaluations
 *
 * Polynomials were obtained using Sollya scripts (in comments below)
 *
 *
*/

/*
f= atan((1-x)/(1+x))-atan(1);
I=[-1+10^(-4);1.0];
filename="atan.txt";
print("") > filename;
for deg from 3 to 11 do begin
  p = fpminimax(f, deg,[|1,23...|],I, floating, absolute); 
  display=decimal;
  acc=floor(-log2(sup(supnorm(p, f, I, absolute, 2^(-20)))));
  print( "   // degree = ", deg, 
         "  => absolute accuracy is ",  acc, "bits" ) >> filename;
end;
*/

#include <cstdio>
#include <cmath>
#include <limits>
#include <chrono>

// float

template <int DEGREE>

// degree =  5   => absolute accuracy is  10 bits
template <>
constexpr float approx_atan2f_P<5>(float x) {
  auto z = x * x;
  return x * (float(-0xf.ecfc8p-4) + z * (float(0x4.9e79dp-4) + z * float(-0x1.44f924p-4)));
}

// degree =  7   => absolute accuracy is  13 bits
template <>
constexpr float approx_atan2f_P<7>(float x) {
  auto z = x * x;
  return x * (float(-0xf.fcc7ap-4) + z * (float(0x5.23886p-4) + z * (float(-0x2.571968p-4) + z * float(0x9.fb05p-8))));
}

// degree =  9   => absolute accuracy is  16 bits
template <>
constexpr float approx_atan2f_P<9>(float x) {
  auto z = x * x;
  return x * (float(-0xf.ff73ep-4) +
              z * (float(0x5.48ee1p-4) +
                   z * (float(-0x2.e1efe8p-4) + z * (float(0x1.5cce54p-4) + z * float(-0x5.56245p-8)))));
}

// degree =  11   => absolute accuracy is  19 bits
template <>
constexpr float approx_atan2f_P<11>(float x) {
  auto z = x * x;
  return x * (float(-0xf.ffe82p-4) +
              z * (float(0x5.526c8p-4) +
                   z * (float(-0x3.18bea8p-4) +
                        z * (float(0x1.dce3bcp-4) + z * (float(-0xd.7a64ap-8) + z * float(0x3.000eap-8))))));
}

// degree =  13   => absolute accuracy is  21 bits
template <>
constexpr float approx_atan2f_P<13>(float x) {
  auto z = x * x;
  return x * (float(-0xf.fffbep-4) +
              z * (float(0x5.54adp-4) +
                   z * (float(-0x3.2b4df8p-4) +
                        z * (float(0x2.1df79p-4) +
                             z * (float(-0x1.46081p-4) + z * (float(0x8.99028p-8) + z * float(-0x1.be0bc4p-8)))))));
}

// degree =  15   => absolute accuracy is  24 bits
template <>
constexpr float approx_atan2f_P<15>(float x) {
  auto z = x * x;
  return x * (float(-0xf.ffff4p-4) +
              z * (float(0x5.552f9p-4 + z * (float(-0x3.30f728p-4) +
                                             z * (float(0x2.39826p-4) +
                                                  z * (float(-0x1.8a880cp-4) +
                                                       z * (float(0xe.484d6p-8) +
                                                            z * (float(-0x5.93d5p-8) + z * float(0x1.0875dcp-8)))))))));
}

template <int DEGREE>

template <int DEGREE>

template <int DEGREE>

// integer...
/*
  f= (2^31/pi)*(atan((1-x)/(1+x))-atan(1));
  I=[-1+10^(-4);1.0];
  p = fpminimax(f, [|1,3,5,7,9,11|],[|23...|],I, floating, absolute);
 */

template <int DEGREE>

// degree =  5   => absolute accuracy is  4*10^5
template <>
constexpr float approx_atan2i_P<5>(float x) {
  auto z = x * x;
  return x * (-680392064.f + z * (197338400.f + z * (-54233256.f)));
}

// degree =  7   => absolute accuracy is  6*10^4
template <>
constexpr float approx_atan2i_P<7>(float x) {
  auto z = x * x;
  return x * (-683027840.f + z * (219543904.f + z * (-99981040.f + z * 26649684.f)));
}

// degree =  9   => absolute accuracy is  8000
template <>
constexpr float approx_atan2i_P<9>(float x) {
  auto z = x * x;
  return x * (-683473920.f + z * (225785056.f + z * (-123151184.f + z * (58210592.f + z * (-14249276.f)))));
}

// degree =  11   => absolute accuracy is  1000
template <>
constexpr float approx_atan2i_P<11>(float x) {
  auto z = x * x;
  return x *
         (-683549696.f + z * (227369312.f + z * (-132297008.f + z * (79584144.f + z * (-35987016.f + z * 8010488.f)))));
}

// degree =  13   => absolute accuracy is  163
template <>
constexpr float approx_atan2i_P<13>(float x) {
  auto z = x * x;
  return x * (-683562624.f +
              z * (227746080.f +
                   z * (-135400128.f + z * (90460848.f + z * (-54431464.f + z * (22973256.f + z * (-4657049.f)))))));
}

template <>
constexpr float approx_atan2i_P<15>(float x) {
  auto z = x * x;
  return x * (-683562624.f +
              z * (227746080.f +
                   z * (-135400128.f + z * (90460848.f + z * (-54431464.f + z * (22973256.f + z * (-4657049.f)))))));
}

template <int DEGREE>

template <int DEGREE>

// short (16bits)
template <int DEGREE>

// degree =  5   => absolute accuracy is  7
template <>
constexpr float approx_atan2s_P<5>(float x) {
  auto z = x * x;
  return x * ((-10381.9609375f) + z * ((3011.1513671875f) + z * (-827.538330078125f)));
}

// degree =  7   => absolute accuracy is  2
template <>
constexpr float approx_atan2s_P<7>(float x) {
  auto z = x * x;
  return x * ((-10422.177734375f) + z * (3349.97412109375f + z * ((-1525.589599609375f) + z * 406.64190673828125f)));
}

// degree =  9   => absolute accuracy is 1
template <>
constexpr float approx_atan2s_P<9>(float x) {
  auto z = x * x;
  return x * ((-10428.984375f) + z * (3445.20654296875f + z * ((-1879.137939453125f) +
                                                               z * (888.22314453125f + z * (-217.42669677734375f)))));
}

template <int DEGREE>

template <int DEGREE>







