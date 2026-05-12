/*
 * Stub header for Xilinx hls_math.h
 * For compilation testing with clang++ only - NOT for HLS synthesis
 *
 * Wraps standard <cmath> functions in the hls:: namespace
 * to allow HLS code that uses hls::sqrt, hls::exp, etc. to compile.
 */

#ifndef __HLS_MATH_H__
#define __HLS_MATH_H__

#include <cmath>
#include <cstdlib>

namespace hls {

// Basic math functions
inline float sqrt(float x) { return std::sqrt(x); }
inline double sqrt(double x) { return std::sqrt(x); }

inline float cbrt(float x) { return std::cbrt(x); }
inline double cbrt(double x) { return std::cbrt(x); }

inline float exp(float x) { return std::exp(x); }
inline double exp(double x) { return std::exp(x); }

inline float exp2(float x) { return std::exp2(x); }
inline double exp2(double x) { return std::exp2(x); }

inline float log(float x) { return std::log(x); }
inline double log(double x) { return std::log(x); }

inline float log2(float x) { return std::log2(x); }
inline double log2(double x) { return std::log2(x); }

inline float log10(float x) { return std::log10(x); }
inline double log10(double x) { return std::log10(x); }

inline float pow(float base, float exp) { return std::pow(base, exp); }
inline double pow(double base, double exp) { return std::pow(base, exp); }

inline float fabs(float x) { return std::fabs(x); }
inline double fabs(double x) { return std::fabs(x); }

inline float abs(float x) { return std::fabs(x); }
inline double abs(double x) { return std::fabs(x); }
inline int abs(int x) { return std::abs(x); }

// Trigonometric functions
inline float sin(float x) { return std::sin(x); }
inline double sin(double x) { return std::sin(x); }

inline float cos(float x) { return std::cos(x); }
inline double cos(double x) { return std::cos(x); }

inline float tan(float x) { return std::tan(x); }
inline double tan(double x) { return std::tan(x); }

inline float asin(float x) { return std::asin(x); }
inline double asin(double x) { return std::asin(x); }

inline float acos(float x) { return std::acos(x); }
inline double acos(double x) { return std::acos(x); }

inline float atan(float x) { return std::atan(x); }
inline double atan(double x) { return std::atan(x); }

inline float atan2(float y, float x) { return std::atan2(y, x); }
inline double atan2(double y, double x) { return std::atan2(y, x); }

// Hyperbolic functions
inline float sinh(float x) { return std::sinh(x); }
inline double sinh(double x) { return std::sinh(x); }

inline float cosh(float x) { return std::cosh(x); }
inline double cosh(double x) { return std::cosh(x); }

inline float tanh(float x) { return std::tanh(x); }
inline double tanh(double x) { return std::tanh(x); }

// Rounding
inline float ceil(float x) { return std::ceil(x); }
inline double ceil(double x) { return std::ceil(x); }

inline float floor(float x) { return std::floor(x); }
inline double floor(double x) { return std::floor(x); }

inline float round(float x) { return std::round(x); }
inline double round(double x) { return std::round(x); }

inline float trunc(float x) { return std::trunc(x); }
inline double trunc(double x) { return std::trunc(x); }

// Min/Max
inline float fmax(float x, float y) { return std::fmax(x, y); }
inline double fmax(double x, double y) { return std::fmax(x, y); }

inline float fmin(float x, float y) { return std::fmin(x, y); }
inline double fmin(double x, double y) { return std::fmin(x, y); }

// Reciprocal (HLS-specific)
inline float recip(float x) { return 1.0f / x; }
inline double recip(double x) { return 1.0 / x; }

inline float rsqrt(float x) { return 1.0f / std::sqrt(x); }
inline double rsqrt(double x) { return 1.0 / std::sqrt(x); }

// half-precision placeholders
typedef float half;

} // namespace hls

#endif // __HLS_MATH_H__
