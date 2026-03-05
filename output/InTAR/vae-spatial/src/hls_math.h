/* Stub header for HLS math functions */
#ifndef HLS_MATH_H
#define HLS_MATH_H

#include <cmath>

// HLS math functions are mostly the same as standard C++ math
namespace hls {
    using std::exp;
    using std::log;
    using std::log10;
    using std::sqrt;
    using std::pow;
    using std::sin;
    using std::cos;
    using std::tan;
    using std::asin;
    using std::acos;
    using std::atan;
    using std::atan2;
    using std::sinh;
    using std::cosh;
    using std::tanh;
    using std::ceil;
    using std::floor;
    using std::trunc;
    using std::round;
    using std::fmod;
    using std::fabs;
}

// For Vitis HLS compatibility
using hls::exp;
using hls::log;
using hls::log10;
using hls::sqrt;
using hls::pow;
using hls::sin;
using hls::cos;
using hls::tan;
using hls::asin;
using hls::acos;
using hls::atan;
using hls::atan2;
using hls::sinh;
using hls::cosh;
using hls::tanh;
using hls::ceil;
using hls::floor;
using hls::trunc;
using hls::round;
using hls::fmod;
using hls::fabs;

#endif // HLS_MATH_H
