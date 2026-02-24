/*
 * Stub header for Xilinx ap_fixed.h
 * For compilation testing with clang++ only - NOT for HLS synthesis
 *
 * This provides minimal type definitions to allow code using ap_fixed/ap_ufixed
 * to compile with standard compilers.
 */

#ifndef __AP_FIXED_H__
#define __AP_FIXED_H__

#include <cstdint>
#include <cmath>

// Arbitrary-precision signed fixed-point
// W = total width, I = integer bits, Q = quantization mode, O = overflow mode
template<int W, int I, int Q = 0, int O = 0>
class ap_fixed {
public:
    double val;  // Use double for simulation

    ap_fixed() : val(0) {}
    ap_fixed(double v) : val(v) {}
    ap_fixed(float v) : val(v) {}
    ap_fixed(int v) : val(v) {}
    ap_fixed(long v) : val(v) {}
    ap_fixed(long long v) : val(v) {}

    operator double() const { return val; }
    operator float() const { return static_cast<float>(val); }
    operator int() const { return static_cast<int>(val); }
    operator long long() const { return static_cast<long long>(val); }
    operator short() const { return static_cast<short>(val); }
    operator unsigned int() const { return static_cast<unsigned int>(val); }
    operator unsigned long long() const { return static_cast<unsigned long long>(val); }
    operator unsigned short() const { return static_cast<unsigned short>(val); }

    ap_fixed operator+(const ap_fixed& other) const { return ap_fixed(val + other.val); }
    ap_fixed operator-(const ap_fixed& other) const { return ap_fixed(val - other.val); }
    ap_fixed operator*(const ap_fixed& other) const { return ap_fixed(val * other.val); }
    ap_fixed operator/(const ap_fixed& other) const { return ap_fixed(val / other.val); }
    ap_fixed operator-() const { return ap_fixed(-val); }

    ap_fixed& operator+=(const ap_fixed& other) { val += other.val; return *this; }
    ap_fixed& operator-=(const ap_fixed& other) { val -= other.val; return *this; }
    ap_fixed& operator*=(const ap_fixed& other) { val *= other.val; return *this; }
    ap_fixed& operator/=(const ap_fixed& other) { val /= other.val; return *this; }

    bool operator==(const ap_fixed& other) const { return val == other.val; }
    bool operator!=(const ap_fixed& other) const { return val != other.val; }
    bool operator<(const ap_fixed& other) const { return val < other.val; }
    bool operator>(const ap_fixed& other) const { return val > other.val; }
    bool operator<=(const ap_fixed& other) const { return val <= other.val; }
    bool operator>=(const ap_fixed& other) const { return val >= other.val; }

    // Mixed operations with double
    ap_fixed operator+(double other) const { return ap_fixed(val + other); }
    ap_fixed operator-(double other) const { return ap_fixed(val - other); }
    ap_fixed operator*(double other) const { return ap_fixed(val * other); }
    ap_fixed operator/(double other) const { return ap_fixed(val / other); }
};

// Arbitrary-precision unsigned fixed-point
template<int W, int I, int Q = 0, int O = 0>
class ap_ufixed {
public:
    double val;

    ap_ufixed() : val(0) {}
    ap_ufixed(double v) : val(std::abs(v)) {}
    ap_ufixed(float v) : val(std::abs(v)) {}
    ap_ufixed(unsigned int v) : val(v) {}
    ap_ufixed(unsigned long v) : val(v) {}
    ap_ufixed(unsigned long long v) : val(v) {}
    ap_ufixed(int v) : val(std::abs(v)) {}
    ap_ufixed(long long v) : val(std::abs(v)) {}

    operator double() const { return val; }
    operator float() const { return static_cast<float>(val); }
    operator int() const { return static_cast<int>(val); }
    operator long long() const { return static_cast<long long>(val); }
    operator short() const { return static_cast<short>(val); }
    operator unsigned int() const { return static_cast<unsigned int>(val); }
    operator unsigned long long() const { return static_cast<unsigned long long>(val); }
    operator unsigned short() const { return static_cast<unsigned short>(val); }

    ap_ufixed operator+(const ap_ufixed& other) const { return ap_ufixed(val + other.val); }
    ap_ufixed operator-(const ap_ufixed& other) const { return ap_ufixed(val - other.val); }
    ap_ufixed operator*(const ap_ufixed& other) const { return ap_ufixed(val * other.val); }
    ap_ufixed operator/(const ap_ufixed& other) const { return ap_ufixed(val / other.val); }

    ap_ufixed& operator+=(const ap_ufixed& other) { val += other.val; return *this; }
    ap_ufixed& operator-=(const ap_ufixed& other) { val -= other.val; return *this; }
    ap_ufixed& operator*=(const ap_ufixed& other) { val *= other.val; return *this; }
    ap_ufixed& operator/=(const ap_ufixed& other) { val /= other.val; return *this; }

    bool operator==(const ap_ufixed& other) const { return val == other.val; }
    bool operator!=(const ap_ufixed& other) const { return val != other.val; }
    bool operator<(const ap_ufixed& other) const { return val < other.val; }
    bool operator>(const ap_ufixed& other) const { return val > other.val; }
    bool operator<=(const ap_ufixed& other) const { return val <= other.val; }
    bool operator>=(const ap_ufixed& other) const { return val >= other.val; }
};

// Quantization modes (stubs)
#define AP_TRN 0
#define AP_RND 1
#define AP_TRN_ZERO 2
#define AP_RND_ZERO 3
#define AP_RND_INF 4
#define AP_RND_MIN_INF 5
#define AP_RND_CONV 6

// Overflow modes (stubs)
#define AP_WRAP 0
#define AP_SAT 1
#define AP_SAT_ZERO 2
#define AP_SAT_SYM 3

#endif // __AP_FIXED_H__
