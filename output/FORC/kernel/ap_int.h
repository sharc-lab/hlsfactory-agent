/*
 * Enhanced stub header for Xilinx ap_int.h
 * For compilation testing with clang++ only - NOT for HLS synthesis
 *
 * This provides minimal type definitions to allow code using ap_int/ap_uint
 * to compile with standard compilers.
 */

#ifndef __AP_INT_H__
#define __AP_INT_H__

#include <cstdint>
#include <limits>

// Forward declarations
template<int W> class ap_int;
template<int W> class ap_uint;

// Range proxy class for assignment to range
template<int W>
class ap_range_proxy {
private:
    typename std::conditional<
        (W <= 8), uint8_t,
        typename std::conditional<
            (W <= 16), uint16_t,
            typename std::conditional<
                (W <= 32), uint32_t,
                uint64_t
            >::type
        >::type
    >::type& parent_val;
    int hi, lo;
    
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
    
    ap_range_proxy(storage_type& v, int h, int l) : parent_val(v), hi(h), lo(l) {}
    
    // Assignment from ap_uint
    template<int W2>
    ap_range_proxy& operator=(const ap_uint<W2>& other) {
        // Simulate range assignment (no-op for stub)
        return *this;
    }
    
    // Assignment from ap_int
    template<int W2>
    ap_range_proxy& operator=(const ap_int<W2>& other) {
        return *this;
    }
    
    // Assignment from integer
    ap_range_proxy& operator=(uint64_t val) {
        return *this;
    }
    
    ap_range_proxy& operator=(int val) {
        return *this;
    }
    
    // Conversion to ap_uint
    operator ap_uint<W>() const;
};

// Arbitrary-precision signed integer
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
    ap_int(unsigned int v) : val(static_cast<storage_type>(v)) {}
    ap_int(unsigned long v) : val(static_cast<storage_type>(v)) {}
    ap_int(unsigned long long v) : val(static_cast<storage_type>(v)) {}

    operator storage_type() const { return val; }
    operator unsigned int() const { return static_cast<unsigned int>(val); }

    ap_int operator+(const ap_int& other) const { return ap_int(val + other.val); }
    ap_int operator-(const ap_int& other) const { return ap_int(val - other.val); }
    ap_int operator*(const ap_int& other) const { return ap_int(val * other.val); }
    ap_int operator/(const ap_int& other) const { return ap_int(val / other.val); }
    ap_int operator%(const ap_int& other) const { return ap_int(val % other.val); }
    ap_int operator&(const ap_int& other) const { return ap_int(val & other.val); }
    ap_int operator|(const ap_int& other) const { return ap_int(val | other.val); }
    ap_int operator^(const ap_int& other) const { return ap_int(val ^ other.val); }
    ap_int operator<<(int shift) const { return ap_int(val << shift); }
    ap_int operator>>(int shift) const { return ap_int(val >> shift); }
    ap_int operator-() const { return ap_int(-val); }

    ap_int& operator+=(const ap_int& other) { val += other.val; return *this; }
    ap_int& operator-=(const ap_int& other) { val -= other.val; return *this; }
    ap_int& operator++() { ++val; return *this; }
    ap_int operator++(int) { ap_int tmp = *this; ++val; return tmp; }
    ap_int& operator&=(const ap_int& other) { val &= other.val; return *this; }
    ap_int& operator|=(const ap_int& other) { val |= other.val; return *this; }
    ap_int& operator^=(const ap_int& other) { val ^= other.val; return *this; }
    ap_int& operator<<=(int shift) { val <<= shift; return *this; }
    ap_int& operator>>=(int shift) { val >>= shift; return *this; }

    bool operator==(const ap_int& other) const { return val == other.val; }
    bool operator!=(const ap_int& other) const { return val != other.val; }
    bool operator<(const ap_int& other) const { return val < other.val; }
    bool operator>(const ap_int& other) const { return val > other.val; }
    bool operator<=(const ap_int& other) const { return val <= other.val; }
    bool operator>=(const ap_int& other) const { return val >= other.val; }

    // Bit access (simplified)
    bool operator[](int i) const { return (val >> i) & 1; }

    // Range access (const) - returns value
    ap_int range(int hi, int lo) const {
        return ap_int((val >> lo) & ((1LL << (hi - lo + 1)) - 1));
    }
    
    // Range access (non-const) - returns proxy for assignment
    ap_range_proxy<W> range(int hi, int lo) {
        return ap_range_proxy<W>(reinterpret_cast<typename ap_range_proxy<W>::storage_type&>(val), hi, lo);
    }
};

// Arbitrary-precision unsigned integer
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
    ap_uint(unsigned int v) : val(static_cast<storage_type>(v)) {}
    ap_uint(unsigned long v) : val(static_cast<storage_type>(v)) {}
    ap_uint(unsigned long long v) : val(static_cast<storage_type>(v)) {}
    ap_uint(int v) : val(static_cast<storage_type>(v)) {}
    ap_uint(long v) : val(static_cast<storage_type>(v)) {}
    ap_uint(long long v) : val(static_cast<storage_type>(v)) {}
    
    // Constructor from ap_int
    template<int W2>
    ap_uint(const ap_int<W2>& other) : val(static_cast<storage_type>(other.val)) {}

    operator storage_type() const { return val; }
    operator int() const { return static_cast<int>(val); }
    operator unsigned int() const { return static_cast<unsigned int>(val); }

    ap_uint operator+(const ap_uint& other) const { return ap_uint(val + other.val); }
    ap_uint operator-(const ap_uint& other) const { return ap_uint(val - other.val); }
    ap_uint operator*(const ap_uint& other) const { return ap_uint(val * other.val); }
    ap_uint operator/(const ap_uint& other) const { return ap_uint(val / other.val); }
    ap_uint operator%(const ap_uint& other) const { return ap_uint(val % other.val); }
    ap_uint operator&(const ap_uint& other) const { return ap_uint(val & other.val); }
    ap_uint operator|(const ap_uint& other) const { return ap_uint(val | other.val); }
    ap_uint operator^(const ap_uint& other) const { return ap_uint(val ^ other.val); }
    ap_uint operator<<(int shift) const { return ap_uint(val << shift); }
    ap_uint operator>>(int shift) const { return ap_uint(val >> shift); }
    ap_uint operator~() const { return ap_uint(~val); }

    ap_uint& operator+=(const ap_uint& other) { val += other.val; return *this; }
    ap_uint& operator-=(const ap_uint& other) { val -= other.val; return *this; }
    ap_uint& operator++() { ++val; return *this; }
    ap_uint operator++(int) { ap_uint tmp = *this; ++val; return tmp; }
    ap_uint& operator&=(const ap_uint& other) { val &= other.val; return *this; }
    ap_uint& operator|=(const ap_uint& other) { val |= other.val; return *this; }
    ap_uint& operator^=(const ap_uint& other) { val ^= other.val; return *this; }
    ap_uint& operator<<=(int shift) { val <<= shift; return *this; }
    ap_uint& operator>>=(int shift) { val >>= shift; return *this; }

    bool operator==(const ap_uint& other) const { return val == other.val; }
    bool operator!=(const ap_uint& other) const { return val != other.val; }
    bool operator<(const ap_uint& other) const { return val < other.val; }
    bool operator>(const ap_uint& other) const { return val > other.val; }
    bool operator<=(const ap_uint& other) const { return val <= other.val; }
    bool operator>=(const ap_uint& other) const { return val >= other.val; }

    bool operator[](int i) const { return (val >> i) & 1; }

    // Const range - returns value
    ap_uint range(int hi, int lo) const {
        return ap_uint((val >> lo) & ((1ULL << (hi - lo + 1)) - 1));
    }
    
    // Non-const range - returns proxy for assignment
    ap_range_proxy<W> range(int hi, int lo) {
        return ap_range_proxy<W>(val, hi, lo);
    }
    
    // Bit length
    int length() const { return W; }
};

// Conversion operator for range_proxy to ap_uint
template<int W>
ap_range_proxy<W>::operator ap_uint<W>() const {
    return ap_uint<W>((parent_val >> lo) & ((1ULL << (hi - lo + 1)) - 1));
}

// Non-member operators for mixed types
template<int W>
ap_uint<W> operator&(const ap_uint<W>& a, uint64_t b) { return a & ap_uint<W>(b); }

template<int W>
ap_uint<W> operator&(uint64_t a, const ap_uint<W>& b) { return ap_uint<W>(a) & b; }

template<int W>
ap_uint<W> operator|(const ap_uint<W>& a, uint64_t b) { return a | ap_uint<W>(b); }

template<int W>
ap_uint<W> operator|(uint64_t a, const ap_uint<W>& b) { return ap_uint<W>(a) | b; }

template<int W1, int W2>
ap_uint<(W1 > W2 ? W1 : W2)> operator+(const ap_uint<W1>& a, const ap_uint<W2>& b) {
    return ap_uint<(W1 > W2 ? W1 : W2)>(static_cast<uint64_t>(a) + static_cast<uint64_t>(b));
}

#endif // __AP_INT_H__
