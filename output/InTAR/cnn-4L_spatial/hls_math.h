/*
 * Stub header for Xilinx hls_math.h
 * For compilation testing with clang++ only - NOT for HLS synthesis
 *
 * This provides minimal math function definitions to allow code using HLS math
 * to compile with standard compilers.
 */

#ifndef __HLS_MATH_H__
#define __HLS_MATH_H__

#include <cmath>

namespace hls {

// Exponential function
inline double exp(double x) {
    return std::exp(x);
}

inline float exp(float x) {
    return std::exp(x);
}

// Natural logarithm
inline double log(double x) {
    return std::log(x);
}

inline float log(float x) {
    return std::log(x);
}

// Log base 2
inline double log2(double x) {
    return std::log2(x);
}

inline float log2(float x) {
    return std::log2(x);
}

// Square root
inline double sqrt(double x) {
    return std::sqrt(x);
}

inline float sqrt(float x) {
    return std::sqrt(x);
}

// Power
inline double pow(double x, double y) {
    return std::pow(x, y);
}

inline float pow(float x, float y) {
    return std::pow(x, y);
}

// Sine
inline double sin(double x) {
    return std::sin(x);
}

inline float sin(float x) {
    return std::sin(x);
}

// Cosine
inline double cos(double x) {
    return std::cos(x);
}

inline float cos(float x) {
    return std::cos(x);
}

// Tangent
inline double tan(double x) {
    return std::tan(x);
}

inline float tan(float x) {
    return std::tan(x);
}

// Floor
inline double floor(double x) {
    return std::floor(x);
}

inline float floor(float x) {
    return std::floor(x);
}

// Ceiling
inline double ceil(double x) {
    return std::ceil(x);
}

inline float ceil(float x) {
    return std::ceil(x);
}

// Absolute value
inline double fabs(double x) {
    return std::fabs(x);
}

inline float fabs(float x) {
    return std::fabs(x);
}

// Reciprocal square root
inline double rsqrt(double x) {
    return 1.0 / std::sqrt(x);
}

inline float rsqrt(float x) {
    return 1.0f / std::sqrt(x);
}

// Fused multiply-add
inline double fma(double a, double b, double c) {
    return a * b + c;
}

inline float fma(float a, float b, float c) {
    return a * b + c;
}

} // namespace hls

#endif // __HLS_MATH_H__
