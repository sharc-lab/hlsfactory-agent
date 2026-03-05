#ifndef AP_FIXED_H_
#define AP_FIXED_H_

#include "ap_int.h"

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

#include <cmath>

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
    ap_fixed(ap_int<W> v) : val((double)(typename ap_int<W>::storage_type)(v)) {}
    
    template<int W2, int I2, int Q2, int O2>
    ap_fixed(const ap_fixed<W2, I2, Q2, O2>& other) : val(other.val) {}
    
    template<int W2, int I2, int Q2, int O2>
    ap_fixed(const ap_ufixed<W2, I2, Q2, O2>& other) : val(other.val) {}
    
    operator double() const { return val; }
    
    ap_fixed operator+(const ap_fixed& other) const { return ap_fixed(val + other.val); }
    ap_fixed operator-(const ap_fixed& other) const { return ap_fixed(val - other.val); }
    ap_fixed operator*(const ap_fixed& other) const { return ap_fixed(val * other.val); }
    ap_fixed operator/(const ap_fixed& other) const { return ap_fixed(val / other.val); }
    
    ap_fixed operator+=(const ap_fixed& other) { val += other.val; return *this; }
    ap_fixed operator-=(const ap_fixed& other) { val -= other.val; return *this; }
    ap_fixed operator*=(const ap_fixed& other) { val *= other.val; return *this; }
    ap_fixed operator/=(const ap_fixed& other) { val /= other.val; return *this; }
    
    ap_fixed operator-() const { return ap_fixed(-val); }
};

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
    ap_ufixed(ap_uint<W> v) : val((double)(typename ap_uint<W>::storage_type)(v)) {}
    
    template<int W2, int I2, int Q2, int O2>
    ap_ufixed(const ap_fixed<W2, I2, Q2, O2>& other) : val(other.val) {}
    
    template<int W2, int I2, int Q2, int O2>
    ap_ufixed(const ap_ufixed<W2, I2, Q2, O2>& other) : val(other.val) {}
    
    operator double() const { return val; }
};

#endif // AP_FIXED_H_
