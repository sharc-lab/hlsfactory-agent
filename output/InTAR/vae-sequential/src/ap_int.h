/*
 * Stub header for Xilinx ap_int.h
 * For compilation testing with clang++ only - NOT for HLS synthesis
 */
#ifndef __AP_INT_H__
#define __AP_INT_H__

#include <cstdint>
#include <limits>

template<int W>
class ap_int {
public:
    using storage_type = typename std::conditional<
        (W <= 8), int8_t,
        typename std::conditional<
            (W <= 16), int16_t,
            typename std::conditional<
                (W <= 32), int32_t,
                int64_t
            >::type
        >::type
    >::type;

    storage_type val;
    
    ap_int() : val(0) {}
    ap_int(int v) : val(static_cast<storage_type>(v)) {}
    ap_int(long v) : val(static_cast<storage_type>(v)) {}
    ap_int(long long v) : val(static_cast<storage_type>(v)) {}
    
    template<int W2>
    ap_int(const ap_int<W2>& other) : val(static_cast<storage_type>(other.val)) {}
    
    ap_int& operator=(int v) { val = static_cast<storage_type>(v); return *this; }
    ap_int& operator=(const ap_int&) = default;
    
    ap_int operator+(const ap_int& other) const { return ap_int(static_cast<int>(val + other.val)); }
    ap_int operator-(const ap_int& other) const { return ap_int(static_cast<int>(val - other.val)); }
    ap_int operator*(const ap_int& other) const { return ap_int(static_cast<int>(val * other.val)); }
    ap_int operator/(const ap_int& other) const { return ap_int(static_cast<int>(val / other.val)); }
    ap_int operator%(const ap_int& other) const { return ap_int(static_cast<int>(val % other.val)); }
    ap_int operator&(const ap_int& other) const { return ap_int(static_cast<int>(val & other.val)); }
    ap_int operator|(const ap_int& other) const { return ap_int(static_cast<int>(val | other.val)); }
    ap_int operator^(const ap_int& other) const { return ap_int(static_cast<int>(val ^ other.val)); }
    ap_int operator<<(int sh) const { return ap_int(static_cast<int>(val << sh)); }
    ap_int operator>>(int sh) const { return ap_int(static_cast<int>(val >> sh)); }
    ap_int operator-() const { return ap_int(static_cast<int>(-val)); }
    ap_int operator~() const { return ap_int(static_cast<int>(~val)); }
    
    ap_int& operator+=(const ap_int& other) { val += other.val; return *this; }
    ap_int& operator-=(const ap_int& other) { val -= other.val; return *this; }
    ap_int& operator*=(const ap_int& other) { val *= other.val; return *this; }
    ap_int& operator/=(const ap_int& other) { val /= other.val; return *this; }
    ap_int& operator&=(const ap_int& other) { val &= other.val; return *this; }
    ap_int& operator|=(const ap_int& other) { val |= other.val; return *this; }
    ap_int& operator^=(const ap_int& other) { val ^= other.val; return *this; }
    ap_int& operator<<=(int sh) { val <<= sh; return *this; }
    ap_int& operator>>=(int sh) { val >>= sh; return *this; }
    
    bool operator==(const ap_int& other) const { return val == other.val; }
    bool operator!=(const ap_int& other) const { return val != other.val; }
    bool operator<(const ap_int& other) const { return val < other.val; }
    bool operator>(const ap_int& other) const { return val > other.val; }
    bool operator<=(const ap_int& other) const { return val <= other.val; }
    bool operator>=(const ap_int& other) const { return val >= other.val; }
    
    // Explicit comparison with int to avoid ambiguity
    bool operator==(int other) const { return val == other; }
    bool operator!=(int other) const { return val != other; }
    bool operator<(int other) const { return val < other; }
    bool operator>(int other) const { return val > other; }
    bool operator<=(int other) const { return val <= other; }
    bool operator>=(int other) const { return val >= other; }
    
    operator int() const { return static_cast<int>(val); }
    operator long() const { return static_cast<long>(val); }
    operator long long() const { return static_cast<long long>(val); }
    operator float() const { return static_cast<float>(val); }
    operator double() const { return static_cast<double>(val); }
};

template<int W>
class ap_uint {
public:
    using storage_type = typename std::conditional<
        (W <= 8), uint8_t,
        typename std::conditional<
            (W <= 16), uint16_t,
            typename std::conditional<
                (W <= 32), uint32_t,
                uint64_t
            >::type
        >::type
    >::type;

    storage_type val;
    
    ap_uint() : val(0) {}
    ap_uint(unsigned v) : val(static_cast<storage_type>(v)) {}
    ap_uint(unsigned long v) : val(static_cast<storage_type>(v)) {}
    ap_uint(unsigned long long v) : val(static_cast<storage_type>(v)) {}
    
    template<int W2>
    ap_uint(const ap_uint<W2>& other) : val(static_cast<storage_type>(other.val)) {}
    
    ap_uint& operator=(unsigned v) { val = static_cast<storage_type>(v); return *this; }
    ap_uint& operator=(const ap_uint&) = default;
    
    ap_uint operator+(const ap_uint& other) const { return ap_uint(static_cast<unsigned>(val + other.val)); }
    ap_uint operator-(const ap_uint& other) const { return ap_uint(static_cast<unsigned>(val - other.val)); }
    ap_uint operator*(const ap_uint& other) const { return ap_uint(static_cast<unsigned>(val * other.val)); }
    ap_uint operator/(const ap_uint& other) const { return ap_uint(static_cast<unsigned>(val / other.val)); }
    ap_uint operator%(const ap_uint& other) const { return ap_uint(static_cast<unsigned>(val % other.val)); }
    ap_uint operator&(const ap_uint& other) const { return ap_uint(static_cast<unsigned>(val & other.val)); }
    ap_uint operator|(const ap_uint& other) const { return ap_uint(static_cast<unsigned>(val | other.val)); }
    ap_uint operator^(const ap_uint& other) const { return ap_uint(static_cast<unsigned>(val ^ other.val)); }
    ap_uint operator<<(int sh) const { return ap_uint(static_cast<unsigned>(val << sh)); }
    ap_uint operator>>(int sh) const { return ap_uint(static_cast<unsigned>(val >> sh)); }
    ap_uint operator~() const { return ap_uint(static_cast<unsigned>(~val)); }
    
    ap_uint& operator+=(const ap_uint& other) { val += other.val; return *this; }
    ap_uint& operator-=(const ap_uint& other) { val -= other.val; return *this; }
    ap_uint& operator*=(const ap_uint& other) { val *= other.val; return *this; }
    ap_uint& operator/=(const ap_uint& other) { val /= other.val; return *this; }
    ap_uint& operator&=(const ap_uint& other) { val &= other.val; return *this; }
    ap_uint& operator|=(const ap_uint& other) { val |= other.val; return *this; }
    ap_uint& operator^=(const ap_uint& other) { val ^= other.val; return *this; }
    ap_uint& operator<<=(int sh) { val <<= sh; return *this; }
    ap_uint& operator>>=(int sh) { val >>= sh; return *this; }
    
    bool operator==(const ap_uint& other) const { return val == other.val; }
    bool operator!=(const ap_uint& other) const { return val != other.val; }
    bool operator<(const ap_uint& other) const { return val < other.val; }
    bool operator>(const ap_uint& other) const { return val > other.val; }
    bool operator<=(const ap_uint& other) const { return val <= other.val; }
    bool operator>=(const ap_uint& other) const { return val >= other.val; }
    
    // Explicit comparison with unsigned to avoid ambiguity
    bool operator==(unsigned other) const { return val == other; }
    bool operator!=(unsigned other) const { return val != other; }
    bool operator<(unsigned other) const { return val < other; }
    bool operator>(unsigned other) const { return val > other; }
    bool operator<=(unsigned other) const { return val <= other; }
    bool operator>=(unsigned other) const { return val >= other; }
    
    operator unsigned() const { return static_cast<unsigned>(val); }
    operator unsigned long() const { return static_cast<unsigned long>(val); }
    operator unsigned long long() const { return static_cast<unsigned long long>(val); }
};

#endif // __AP_INT_H__
