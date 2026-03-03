/*
 * Stub header for Xilinx ap_int.h
 * For compilation testing with clang++ only - NOT for HLS synthesis
 */

#ifndef __AP_INT_H__
#define __AP_INT_H__

#include <cstdint>
#include <cstring>
#include <string>

template<int _AP_W>
class ap_uint {
private:
    uint8_t data[(_AP_W + 7) / 8];

public:
    static const int width = _AP_W;
    
    ap_uint() { clear(); }
    ap_uint(int val) { *this = val; }
    ap_uint(long val) { *this = val; }
    ap_uint(unsigned val) { *this = val; }
    ap_uint(unsigned long val) { *this = val; }
    ap_uint(unsigned long long val) { *this = val; }
    ap_uint(const char* val) { *this = std::stoull(val); }
    
    template<int _AP_W2>
    ap_uint(const ap_uint<_AP_W2>& other) {
        *this = other.operator unsigned long long();
    }
    
    void clear() { memset(data, 0, sizeof(data)); }
    
    ap_uint& operator=(int val) {
        clear();
        *reinterpret_cast<int*>(data) = val;
        return *this;
    }
    
    ap_uint& operator=(unsigned val) {
        clear();
        *reinterpret_cast<unsigned*>(data) = val;
        return *this;
    }
    
    ap_uint& operator=(unsigned long long val) {
        clear();
        *reinterpret_cast<unsigned long long*>(data) = val;
        return *this;
    }
    
    operator int() const { return *reinterpret_cast<const int*>(data); }
    operator unsigned int() const { return *reinterpret_cast<const unsigned int*>(data); }
    operator unsigned long() const { return *reinterpret_cast<const unsigned long*>(data); }
    operator unsigned long long() const { return *reinterpret_cast<const unsigned long long*>(data); }
    operator float() const { return static_cast<float>(operator unsigned long long()); }
    operator double() const { return static_cast<double>(operator unsigned long long()); }
    
    // Comparison operators
    bool operator<(const ap_uint& other) const {
        return operator unsigned long long() < other.operator unsigned long long();
    }
    bool operator>(const ap_uint& other) const {
        return operator unsigned long long() > other.operator unsigned long long();
    }
    bool operator==(const ap_uint& other) const {
        return operator unsigned long long() == other.operator unsigned long long();
    }
    bool operator!=(const ap_uint& other) const {
        return operator unsigned long long() != other.operator unsigned long long();
    }
    bool operator<=(const ap_uint& other) const {
        return operator unsigned long long() <= other.operator unsigned long long();
    }
    bool operator>=(const ap_uint& other) const {
        return operator unsigned long long() >= other.operator unsigned long long();
    }
    
    // Arithmetic operators
    ap_uint operator+(const ap_uint& other) const {
        return operator unsigned long long() + other.operator unsigned long long();
    }
    ap_uint operator-(const ap_uint& other) const {
        return operator unsigned long long() - other.operator unsigned long long();
    }
    ap_uint operator*(const ap_uint& other) const {
        return operator unsigned long long() * other.operator unsigned long long();
    }
    ap_uint operator/(const ap_uint& other) const {
        return operator unsigned long long() / other.operator unsigned long long();
    }
    ap_uint operator%(const ap_uint& other) const {
        return operator unsigned long long() % other.operator unsigned long long();
    }
    
    ap_uint& operator++() {
        *this = operator unsigned long long() + 1;
        return *this;
    }
    
    ap_uint operator++(int) {
        ap_uint old = *this;
        *this = operator unsigned long long() + 1;
        return old;
    }
    
    ap_uint& operator--() {
        *this = operator unsigned long long() - 1;
        return *this;
    }
    
    ap_uint operator--(int) {
        ap_uint old = *this;
        *this = operator unsigned long long() - 1;
        return old;
    }
    
    ap_uint operator+(int val) const {
        return operator unsigned long long() + val;
    }
    ap_uint operator-(int val) const {
        return operator unsigned long long() - val;
    }
    ap_uint operator*(int val) const {
        return operator unsigned long long() * val;
    }
    ap_uint operator/(int val) const {
        return operator unsigned long long() / val;
    }
    
    ap_uint& operator+=(const ap_uint& other) {
        *this = operator unsigned long long() + other.operator unsigned long long();
        return *this;
    }
    ap_uint& operator-=(const ap_uint& other) {
        *this = operator unsigned long long() - other.operator unsigned long long();
        return *this;
    }
    
    // Bitwise operators
    ap_uint operator|(const ap_uint& other) const {
        ap_uint result;
        for (size_t i = 0; i < sizeof(data); i++) {
            result.data[i] = data[i] | other.data[i];
        }
        return result;
    }
    
    ap_uint operator&(const ap_uint& other) const {
        ap_uint result;
        for (size_t i = 0; i < sizeof(data); i++) {
            result.data[i] = data[i] & other.data[i];
        }
        return result;
    }
    
    ap_uint operator^(const ap_uint& other) const {
        ap_uint result;
        for (size_t i = 0; i < sizeof(data); i++) {
            result.data[i] = data[i] ^ other.data[i];
        }
        return result;
    }
    
    ap_uint operator~() const {
        ap_uint result;
        for (size_t i = 0; i < sizeof(data); i++) {
            result.data[i] = ~data[i];
        }
        return result;
    }
    
    ap_uint operator<<(int shift) const {
        return operator unsigned long long() << shift;
    }
    
    ap_uint operator>>(int shift) const {
        return operator unsigned long long() >> shift;
    }
    
    ap_uint& operator<<=(int shift) {
        *this = operator unsigned long long() << shift;
        return *this;
    }
    
    ap_uint& operator>>=(int shift) {
        *this = operator unsigned long long() >> shift;
        return *this;
    }
    
    ap_uint& operator|=(const ap_uint& other) {
        *this = *this | other;
        return *this;
    }
    
    ap_uint& operator&=(const ap_uint& other) {
        *this = *this & other;
        return *this;
    }
    
    // Range method - returns a new ap_uint with bits from high to low
    ap_uint range(int high, int low) const {
        ap_uint result;
        unsigned long long val = operator unsigned long long();
        unsigned long long mask;
        if (high - low + 1 >= 64) {
            mask = ~0ULL;
        } else {
            mask = ((1ULL << (high - low + 1)) - 1);
        }
        result = (val >> low) & mask;
        return result;
    }
    
    // Get bit count
    int length() const { return _AP_W; }
    
    // Concatenation operator (comma operator)
    template<int _AP_W2>
    ap_uint<_AP_W + _AP_W2> operator,(const ap_uint<_AP_W2>& other) const {
        ap_uint<_AP_W + _AP_W2> result;
        unsigned long long lsb = operator unsigned long long();
        unsigned long long msb = other.operator unsigned long long();
        result = (msb << _AP_W) | lsb;
        return result;
    }
    
    template<int _AP_W2>
    ap_uint<_AP_W + _AP_W2> concat(const ap_uint<_AP_W2>& other) const {
        ap_uint<_AP_W + _AP_W2> result;
        unsigned long long lsb = operator unsigned long long();
        unsigned long long msb = other.operator unsigned long long();
        result = (msb << _AP_W) | lsb;
        return result;
    }
    
    // Bit access
    bool operator[](int index) const {
        return (operator unsigned long long() >> index) & 1;
    }
};

// Free function comparison operators to resolve ambiguity
template<int _AP_W>
inline bool operator<(int lhs, const ap_uint<_AP_W>& rhs) {
    return static_cast<unsigned long long>(lhs) < rhs.operator unsigned long long();
}

template<int _AP_W>
inline bool operator>(int lhs, const ap_uint<_AP_W>& rhs) {
    return static_cast<unsigned long long>(lhs) > rhs.operator unsigned long long();
}

template<int _AP_W>
inline bool operator==(int lhs, const ap_uint<_AP_W>& rhs) {
    return static_cast<unsigned long long>(lhs) == rhs.operator unsigned long long();
}

template<int _AP_W>
inline bool operator!=(int lhs, const ap_uint<_AP_W>& rhs) {
    return static_cast<unsigned long long>(lhs) != rhs.operator unsigned long long();
}

template<int _AP_W>
inline bool operator<=(int lhs, const ap_uint<_AP_W>& rhs) {
    return static_cast<unsigned long long>(lhs) <= rhs.operator unsigned long long();
}

template<int _AP_W>
inline bool operator>=(int lhs, const ap_uint<_AP_W>& rhs) {
    return static_cast<unsigned long long>(lhs) >= rhs.operator unsigned long long();
}

// ap_int class
template<int _AP_W>
class ap_int {
private:
    uint8_t data[(_AP_W + 7) / 8];

public:
    ap_int() { clear(); }
    ap_int(int val) { *this = val; }
    ap_int(long val) { *this = val; }
    
    void clear() { memset(data, 0, sizeof(data)); }
    
    ap_int& operator=(int val) {
        clear();
        *reinterpret_cast<int*>(data) = val;
        return *this;
    }
    
    operator int() const { return *reinterpret_cast<const int*>(data); }
    operator long() const { return *reinterpret_cast<const long*>(data); }
    operator long long() const { return *reinterpret_cast<const long long*>(data); }
    operator float() const { return static_cast<float>(operator long long()); }
    operator double() const { return static_cast<double>(operator long long()); }
    
    // Range method
    ap_int range(int high, int low) const {
        ap_int result;
        long long val = operator long long();
        long long mask = ((1LL << (high - low + 1)) - 1);
        result = (val >> low) & mask;
        return result;
    }
};

// Convenience typedefs
using ap_uint1 = ap_uint<1>;
using ap_uint2 = ap_uint<2>;
using ap_uint4 = ap_uint<4>;
using ap_uint8 = ap_uint<8>;
using ap_uint16 = ap_uint<16>;
using ap_uint32 = ap_uint<32>;
using ap_uint64 = ap_uint<64>;
using ap_uint128 = ap_uint<128>;
using ap_uint256 = ap_uint<256>;
using ap_uint512 = ap_uint<512>;
using ap_uint1024 = ap_uint<1024>;

using ap_int1 = ap_int<1>;
using ap_int2 = ap_int<2>;
using ap_int4 = ap_int<4>;
using ap_int8 = ap_int<8>;
using ap_int16 = ap_int<16>;
using ap_int32 = ap_int<32>;
using ap_int64 = ap_int<64>;
using ap_int128 = ap_int<128>;
using ap_int256 = ap_int<256>;
using ap_int512 = ap_int<512>;
using ap_int1024 = ap_int<1024>;

#endif // __AP_INT_H__
