/*
 * Stub header for Xilinx ap_int.h
 * For compilation testing with clang++ only - NOT for HLS synthesis
 *
 * This provides minimal type definitions to allow code using ap_int/ap_uint
 * to compile with standard compilers.
 */

#ifndef __AP_INT_H__
#define __AP_INT_H__

#include <cstdint>
#include <limits>

// Arbitrary-precision signed integer
template<int W>
class ap_int {
public:
    // Use largest standard type that fits
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

    operator storage_type() const { return val; }

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

    ap_int& operator+=(const ap_int& other) { val += other.val; return *this; }
    ap_int& operator-=(const ap_int& other) { val -= other.val; return *this; }
    ap_int& operator++() { ++val; return *this; }
    ap_int operator++(int) { ap_int tmp = *this; ++val; return tmp; }

    bool operator==(const ap_int& other) const { return val == other.val; }
    bool operator!=(const ap_int& other) const { return val != other.val; }
    bool operator<(const ap_int& other) const { return val < other.val; }
    bool operator>(const ap_int& other) const { return val > other.val; }
    bool operator<=(const ap_int& other) const { return val <= other.val; }
    bool operator>=(const ap_int& other) const { return val >= other.val; }

    // Bit access (simplified)
    bool operator[](int i) const { return (val >> i) & 1; }

    // Range access (simplified - returns full value)
    ap_int range(int hi, int lo) const {
        return ap_int((val >> lo) & ((1LL << (hi - lo + 1)) - 1));
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

    operator storage_type() const { return val; }

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

    ap_uint& operator+=(const ap_uint& other) { val += other.val; return *this; }
    ap_uint& operator-=(const ap_uint& other) { val -= other.val; return *this; }
    ap_uint& operator++() { ++val; return *this; }
    ap_uint operator++(int) { ap_uint tmp = *this; ++val; return tmp; }

    bool operator==(const ap_uint& other) const { return val == other.val; }
    bool operator!=(const ap_uint& other) const { return val != other.val; }
    bool operator<(const ap_uint& other) const { return val < other.val; }
    bool operator>(const ap_uint& other) const { return val > other.val; }
    bool operator<=(const ap_uint& other) const { return val <= other.val; }
    bool operator>=(const ap_uint& other) const { return val >= other.val; }

    bool operator[](int i) const { return (val >> i) & 1; }

    ap_uint range(int hi, int lo) const {
        return ap_uint((val >> lo) & ((1ULL << (hi - lo + 1)) - 1));
    }
};

#endif // __AP_INT_H__
