#pragma once

// Xilinx HLS Math Library stub
// This is a stub implementation for compilation testing only

#include <cmath>

// HLS math functions namespace
namespace hls {
    using ::abs;
    using ::acos;
    using ::acosh;
    using ::asin;
    using ::asinh;
    using ::atan;
    using ::atan2;
    using ::atanh;
    using ::cbrt;
    using ::ceil;
    using ::copysign;
    using ::cos;
    using ::cosh;
    using ::erf;
    using ::erfc;
    using ::exp;
    using ::exp2;
    using ::expm1;
    using ::fabs;
    using ::fdim;
    using ::floor;
    using ::fma;
    using ::fmax;
    using ::fmin;
    using ::fmod;
    using ::hypot;
    using ::ilogb;
    using ::ldexp;
    using ::lgamma;
    using ::llrint;
    using ::llround;
    using ::log;
    using ::log10;
    using ::log1p;
    using ::log2;
    using ::logb;
    using ::lrint;
    using ::lround;
    using ::nan;
    using ::nearbyint;
    using ::nextafter;
    using ::pow;
    using ::remainder;
    using ::remquo;
    using ::rint;
    using ::round;
    using ::scalbn;
    using ::sin;
    using ::sinh;
    using ::sqrt;
    using ::tan;
    using ::tanh;
    using ::tgamma;
    using ::trunc;
}

#define hls_fabs fabs
#define hls_sin sin
#define hls_cos cos
#define hls_sqrt sqrt
#define hls_log log
#define hls_exp exp
#define hls_floor floor
#define hls_ceil ceil
#define hls_pow pow
