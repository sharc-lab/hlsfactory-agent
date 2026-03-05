#pragma once

#include <cstdint>
#include <type_traits>
#include <limits>
#include <cassert>
#include <cmath>

// Minimal stub implementation of ap_int for compilation testing

// Define _AP_ROOT_TYPE for compatibility
template<int _AP_W, bool _AP_S>
struct _AP_ROOT_TYPE {
    static const int width = _AP_W;
    static const bool is_signed = _AP_S;
};

// Base class template to provide common functionality
template<int Width, bool Signed>
class ap_int_base {
public:
    static const int width = Width;
    static const bool is_signed = Signed;
    
    struct Base {
        static const int width = Width;
        static const bool is_signed = Signed;
    };
};

template<int Width>
class ap_int : public ap_int_base<Width, true> {
public:
    using storage_type = typename std::conditional<
        (Width <= 8), int8_t,
        typename std::conditional<
            (Width <= 16), int16_t,
            typename std::conditional<
                (Width <= 32), int32_t,
                int64_t
            >::type
        >::type
    >::type;
    
    using Base = ap_int_base<Width, true>;
    
    storage_type val;
    
    ap_int() : val(0) {}
    ap_int(int v) : val(static_cast<storage_type>(v)) {}
    ap_int(long v) : val(static_cast<storage_type>(v)) {}
    ap_int(long long v) : val(static_cast<storage_type>(v)) {}
    
    operator storage_type() const { return val; }
    
    ap_int operator+(const ap_int& other) const { return ap_int(val + other.val); }
    ap_int operator-(const ap_int& other) const { return ap_int(val - other.val); }
    ap_int operator*(const ap_int& other) const { return ap_int(val * other.val); }
    ap_int operator/(const ap_int& other) const { return ap_int(val / other.val); }
    ap_int operator&(const ap_int& other) const { return ap_int(val & other.val); }
    ap_int operator|(const ap_int& other) const { return ap_int(val | other.val); }
    ap_int operator<<(int shift) const { return ap_int(val << shift); }
    ap_int operator>>(int shift) const { return ap_int(val >> shift); }
    ap_int operator-() const { return ap_int(-val); }
    
    bool operator==(const ap_int& other) const { return val == other.val; }
    bool operator!=(const ap_int& other) const { return val != other.val; }
    bool operator<(const ap_int& other) const { return val < other.val; }
    bool operator>(const ap_int& other) const { return val > other.val; }
    
    // Non-template range method for compatibility
    ap_int range(int high, int low) const {
        storage_type mask = ((storage_type(1) << (high - low + 1)) - 1) << low;
        return ap_int((val & mask) >> low);
    }
};

template<int Width>
class ap_uint : public ap_int_base<Width, false> {
public:
    using storage_type = typename std::conditional<
        (Width <= 8), uint8_t,
        typename std::conditional<
            (Width <= 16), uint16_t,
            typename std::conditional<
                (Width <= 32), uint32_t,
                uint64_t
            >::type
        >::type
    >::type;
    
    using Base = ap_int_base<Width, false>;
    
    storage_type val;
    
    ap_uint() : val(0) {}
    ap_uint(unsigned int v) : val(static_cast<storage_type>(v)) {}
    ap_uint(unsigned long v) : val(static_cast<storage_type>(v)) {}
    ap_uint(unsigned long long v) : val(static_cast<storage_type>(v)) {}
    ap_uint(int v) : val(static_cast<storage_type>(v)) {}
    
    operator storage_type() const { return val; }
    
    ap_uint operator+(const ap_uint& other) const { return ap_uint(val + other.val); }
    ap_uint operator-(const ap_uint& other) const { return ap_uint(val - other.val); }
    ap_uint operator*(const ap_uint& other) const { return ap_uint(val * other.val); }
    ap_uint operator/(const ap_uint& other) const { return ap_uint(val / other.val); }
    ap_uint operator&(const ap_uint& other) const { return ap_uint(val & other.val); }
    ap_uint operator|(const ap_uint& other) const { return ap_uint(val | other.val); }
    ap_uint operator<<(int shift) const { return ap_uint(val << shift); }
    ap_uint operator>>(int shift) const { return ap_uint(val >> shift); }
    
    bool operator==(const ap_uint& other) const { return val == other.val; }
    bool operator!=(const ap_uint& other) const { return val != other.val; }
    
    bool operator[](int idx) const { return (val >> idx) & 1; }
    
    // Non-template range method for compatibility
    ap_uint range(int high, int low) const {
        storage_type mask = ((storage_type(1) << (high - low + 1)) - 1);
        return ap_uint((val >> low) & mask);
    }
};

// Rounding modes
#define AP_RND          0
#define AP_RND_ZERO     1
#define AP_RND_MIN_INF  2
#define AP_RND_INF      3
#define AP_RND_CONV     4
#define AP_TRN          5
#define AP_TRN_ZERO     6

// Overflow modes
#define AP_SAT          0
#define AP_SAT_ZERO     1
#define AP_SAT_SYM      2
#define AP_WRAP         3
#define AP_WRAP_SM      4

// ap_fixed stub with 4 template parameters (W, I, Q, O)
template<int W, int I, int Q = AP_TRN, int O = AP_WRAP>
class ap_fixed : public ap_int_base<W, true> {
public:
    double val;
    struct Base {
        static const int width = W;
        static const int iwidth = I;
        static const int qmode = Q;
        static const int omode = O;
        static const bool is_signed = true;
    };
    ap_fixed() : val(0) {}
    ap_fixed(double v) : val(v) {}
    ap_fixed(int v) : val(v) {}
    ap_fixed(long v) : val(v) {}
    
    template<int W2, int I2, int Q2, int O2>
    ap_fixed(const ap_fixed<W2, I2, Q2, O2>& other) : val(other.val) {}
    
    ap_fixed operator+(const ap_fixed& other) const { return ap_fixed(val + other.val); }
    ap_fixed operator-(const ap_fixed& other) const { return ap_fixed(val - other.val); }
    ap_fixed operator*(const ap_fixed& other) const { return ap_fixed(val * other.val); }
    ap_fixed operator/(const ap_fixed& other) const { return ap_fixed(val / other.val); }
    
    ap_fixed operator+=(const ap_fixed& other) { val += other.val; return *this; }
    ap_fixed operator-=(const ap_fixed& other) { val -= other.val; return *this; }
    
    operator double() const { return val; }
};

// ap_ufixed stub with 4 template parameters
template<int W, int I, int Q = AP_TRN, int O = AP_WRAP>
class ap_ufixed : public ap_int_base<W, false> {
public:
    double val;
    struct Base {
        static const int width = W;
        static const int iwidth = I;
        static const int qmode = Q;
        static const int omode = O;
        static const bool is_signed = false;
    };
    ap_ufixed() : val(0) {}
    ap_ufixed(double v) : val(v) {}
    ap_ufixed(unsigned int v) : val(v) {}
    ap_ufixed(unsigned long v) : val(v) {}
    
    template<int W2, int I2, int Q2, int O2>
    ap_ufixed(const ap_ufixed<W2, I2, Q2, O2>& other) : val(other.val) {}
    
    operator double() const { return val; }
};
