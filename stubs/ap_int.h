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
#include <type_traits>

// Forward declarations
template<int W> class ap_int;
template<int W> class ap_uint;

// Range proxy: allows x.range(hi, lo) = y to compile
template<int W>
class ap_range_ref {
    int64_t* parent_val;
    int hi, lo;
public:
    ap_range_ref(int64_t* p, int h, int l) : parent_val(p), hi(h), lo(l) {}

    // Assignment from any integer type
    template<typename T>
    ap_range_ref& operator=(const T& rhs) {
        int64_t mask = ((1LL << (hi - lo + 1)) - 1) << lo;
        *parent_val = (*parent_val & ~mask) | ((static_cast<int64_t>(rhs) << lo) & mask);
        return *this;
    }

    // Assignment from another range_ref
    ap_range_ref& operator=(const ap_range_ref& rhs) {
        int64_t val = (*rhs.parent_val >> rhs.lo) & ((1LL << (rhs.hi - rhs.lo + 1)) - 1);
        return operator=(val);
    }

    // Assignment from ap_int
    template<int W2>
    ap_range_ref& operator=(const ap_int<W2>& rhs);

    // Assignment from ap_uint
    template<int W2>
    ap_range_ref& operator=(const ap_uint<W2>& rhs);

    // Implicit conversion to int64_t for reads
    operator int64_t() const {
        return (*parent_val >> lo) & ((1LL << (hi - lo + 1)) - 1);
    }
};

// Unsigned range proxy
template<int W>
class ap_uint_range_ref {
    uint64_t* parent_val;
    int hi, lo;
public:
    ap_uint_range_ref(uint64_t* p, int h, int l) : parent_val(p), hi(h), lo(l) {}

    template<typename T>
    ap_uint_range_ref& operator=(const T& rhs) {
        uint64_t mask = ((1ULL << (hi - lo + 1)) - 1) << lo;
        *parent_val = (*parent_val & ~mask) | ((static_cast<uint64_t>(rhs) << lo) & mask);
        return *this;
    }

    ap_uint_range_ref& operator=(const ap_uint_range_ref& rhs) {
        uint64_t val = (*rhs.parent_val >> rhs.lo) & ((1ULL << (rhs.hi - rhs.lo + 1)) - 1);
        return operator=(val);
    }

    template<int W2>
    ap_uint_range_ref& operator=(const ap_int<W2>& rhs);

    template<int W2>
    ap_uint_range_ref& operator=(const ap_uint<W2>& rhs);

    operator uint64_t() const {
        return (*parent_val >> lo) & ((1ULL << (hi - lo + 1)) - 1);
    }
};


// Arbitrary-precision signed integer
template<int W>
class ap_int {
public:
    int64_t val;

    ap_int() : val(0) {}
    ap_int(int v) : val(v) {}
    ap_int(unsigned int v) : val(static_cast<int64_t>(v)) {}
    ap_int(long v) : val(v) {}
    ap_int(unsigned long v) : val(static_cast<int64_t>(v)) {}
    ap_int(long long v) : val(v) {}
    ap_int(unsigned long long v) : val(static_cast<int64_t>(v)) {}

    // Cross-width conversion
    template<int W2>
    ap_int(const ap_int<W2>& other) : val(other.val) {}
    template<int W2>
    ap_int(const ap_uint<W2>& other) : val(static_cast<int64_t>(other.val)) {}
    template<int W2>
    ap_int(const ap_range_ref<W2>& other) : val(static_cast<int64_t>(other)) {}
    template<int W2>
    ap_int(const ap_uint_range_ref<W2>& other) : val(static_cast<int64_t>(other)) {}

    operator int() const { return static_cast<int>(val); }
    operator unsigned int() const { return static_cast<unsigned int>(val); }
    operator long() const { return static_cast<long>(val); }
    operator long long() const { return val; }
    operator unsigned long long() const { return static_cast<unsigned long long>(val); }
    operator bool() const { return val != 0; }

    // Conversion methods matching Xilinx API
    int to_int() const { return static_cast<int>(val); }
    long to_long() const { return static_cast<long>(val); }
    long long to_int64() const { return val; }
    unsigned long long to_uint64() const { return static_cast<unsigned long long>(val); }
    double to_double() const { return static_cast<double>(val); }

    // Arithmetic operators (ap_int op ap_int)
    ap_int operator+(const ap_int& other) const { return ap_int(val + other.val); }
    ap_int operator-(const ap_int& other) const { return ap_int(val - other.val); }
    ap_int operator*(const ap_int& other) const { return ap_int(val * other.val); }
    ap_int operator/(const ap_int& other) const { return ap_int(val / other.val); }
    ap_int operator%(const ap_int& other) const { return ap_int(val % other.val); }
    ap_int operator&(const ap_int& other) const { return ap_int(val & other.val); }
    ap_int operator|(const ap_int& other) const { return ap_int(val | other.val); }
    ap_int operator^(const ap_int& other) const { return ap_int(val ^ other.val); }
    ap_int operator~() const { return ap_int(~val); }
    ap_int operator-() const { return ap_int(-val); }
    ap_int operator<<(int shift) const { return ap_int(val << shift); }
    ap_int operator>>(int shift) const { return ap_int(val >> shift); }
    ap_int operator<<(unsigned int shift) const { return ap_int(val << shift); }
    ap_int operator>>(unsigned int shift) const { return ap_int(val >> shift); }

    // Mixed operators with int/unsigned
    ap_int operator+(int other) const { return ap_int(val + other); }
    ap_int operator-(int other) const { return ap_int(val - other); }
    ap_int operator*(int other) const { return ap_int(val * other); }
    ap_int operator/(int other) const { return ap_int(val / other); }
    ap_int operator%(int other) const { return ap_int(val % other); }
    ap_int operator&(int other) const { return ap_int(val & other); }
    ap_int operator|(int other) const { return ap_int(val | other); }

    // Compound assignment
    ap_int& operator+=(const ap_int& other) { val += other.val; return *this; }
    ap_int& operator-=(const ap_int& other) { val -= other.val; return *this; }
    ap_int& operator*=(const ap_int& other) { val *= other.val; return *this; }
    ap_int& operator/=(const ap_int& other) { val /= other.val; return *this; }
    ap_int& operator%=(const ap_int& other) { val %= other.val; return *this; }
    ap_int& operator&=(const ap_int& other) { val &= other.val; return *this; }
    ap_int& operator|=(const ap_int& other) { val |= other.val; return *this; }
    ap_int& operator^=(const ap_int& other) { val ^= other.val; return *this; }
    ap_int& operator<<=(int shift) { val <<= shift; return *this; }
    ap_int& operator>>=(int shift) { val >>= shift; return *this; }

    // Increment/decrement
    ap_int& operator++() { ++val; return *this; }
    ap_int operator++(int) { ap_int tmp = *this; ++val; return tmp; }
    ap_int& operator--() { --val; return *this; }
    ap_int operator--(int) { ap_int tmp = *this; --val; return tmp; }

    // Comparisons
    bool operator==(const ap_int& other) const { return val == other.val; }
    bool operator!=(const ap_int& other) const { return val != other.val; }
    bool operator<(const ap_int& other) const { return val < other.val; }
    bool operator>(const ap_int& other) const { return val > other.val; }
    bool operator<=(const ap_int& other) const { return val <= other.val; }
    bool operator>=(const ap_int& other) const { return val >= other.val; }
    template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    bool operator==(T other) const { return val == static_cast<int64_t>(other); }
    template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    bool operator!=(T other) const { return val != static_cast<int64_t>(other); }
    template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    bool operator<(T other) const { return val < static_cast<int64_t>(other); }
    template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    bool operator>(T other) const { return val > static_cast<int64_t>(other); }
    template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    bool operator<=(T other) const { return val <= static_cast<int64_t>(other); }
    template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    bool operator>=(T other) const { return val >= static_cast<int64_t>(other); }

    // Bit access
    bool operator[](int i) const { return (val >> i) & 1; }

    // Range access — returns proxy that supports assignment
    ap_range_ref<W> range(int hi, int lo) {
        return ap_range_ref<W>(&val, hi, lo);
    }
    int64_t range(int hi, int lo) const {
        return (val >> lo) & ((1LL << (hi - lo + 1)) - 1);
    }
    ap_range_ref<W> operator()(int hi, int lo) {
        return range(hi, lo);
    }
    int64_t operator()(int hi, int lo) const {
        return range(hi, lo);
    }

    // Concat / slice helpers
    int length() const { return W; }
};

// Free functions for int op ap_int
template<int W>
ap_int<W> operator+(int lhs, const ap_int<W>& rhs) { return ap_int<W>(lhs + rhs.val); }
template<int W>
ap_int<W> operator-(int lhs, const ap_int<W>& rhs) { return ap_int<W>(lhs - rhs.val); }
template<int W>
ap_int<W> operator*(int lhs, const ap_int<W>& rhs) { return ap_int<W>(lhs * rhs.val); }
template<int W>
bool operator==(int lhs, const ap_int<W>& rhs) { return lhs == rhs.val; }
template<int W>
bool operator!=(int lhs, const ap_int<W>& rhs) { return lhs != rhs.val; }
template<typename T, int W, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
bool operator<(T lhs, const ap_int<W>& rhs) { return static_cast<int64_t>(lhs) < rhs.val; }
template<typename T, int W, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
bool operator>(T lhs, const ap_int<W>& rhs) { return static_cast<int64_t>(lhs) > rhs.val; }
template<typename T, int W, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
bool operator<=(T lhs, const ap_int<W>& rhs) { return static_cast<int64_t>(lhs) <= rhs.val; }
template<typename T, int W, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
bool operator>=(T lhs, const ap_int<W>& rhs) { return static_cast<int64_t>(lhs) >= rhs.val; }


// Arbitrary-precision unsigned integer
template<int W>
class ap_uint {
public:
    uint64_t val;

    ap_uint() : val(0) {}
    ap_uint(unsigned int v) : val(v) {}
    ap_uint(unsigned long v) : val(v) {}
    ap_uint(unsigned long long v) : val(v) {}
    ap_uint(int v) : val(static_cast<uint64_t>(v)) {}
    ap_uint(long v) : val(static_cast<uint64_t>(v)) {}
    ap_uint(long long v) : val(static_cast<uint64_t>(v)) {}

    // Cross-width conversion
    template<int W2>
    ap_uint(const ap_uint<W2>& other) : val(other.val) {}
    template<int W2>
    ap_uint(const ap_int<W2>& other) : val(static_cast<uint64_t>(other.val)) {}
    template<int W2>
    ap_uint(const ap_range_ref<W2>& other) : val(static_cast<uint64_t>(other)) {}
    template<int W2>
    ap_uint(const ap_uint_range_ref<W2>& other) : val(static_cast<uint64_t>(other)) {}
    template<typename U, int BW = U::bit_width, typename std::enable_if<!std::is_integral<U>::value, int>::type = 0>
    ap_uint(const U& other) {
        U tmp = other;
        ap_uint<BW> packed = static_cast<ap_uint<BW>>(tmp);
        val = static_cast<uint64_t>(packed.val);
    }

    operator unsigned int() const { return static_cast<unsigned int>(val); }
    operator unsigned long() const { return static_cast<unsigned long>(val); }
    operator unsigned long long() const { return val; }
    operator int() const { return static_cast<int>(val); }
    operator long long() const { return static_cast<long long>(val); }
    operator bool() const { return val != 0; }

    // Conversion methods
    int to_int() const { return static_cast<int>(val); }
    unsigned int to_uint() const { return static_cast<unsigned int>(val); }
    long to_long() const { return static_cast<long>(val); }
    long long to_int64() const { return static_cast<long long>(val); }
    unsigned long long to_uint64() const { return val; }
    double to_double() const { return static_cast<double>(val); }

    // Arithmetic operators
    ap_uint operator+(const ap_uint& other) const { return ap_uint(val + other.val); }
    ap_uint operator-(const ap_uint& other) const { return ap_uint(val - other.val); }
    ap_uint operator*(const ap_uint& other) const { return ap_uint(val * other.val); }
    ap_uint operator/(const ap_uint& other) const { return ap_uint(val / other.val); }
    ap_uint operator%(const ap_uint& other) const { return ap_uint(val % other.val); }
    ap_uint operator&(const ap_uint& other) const { return ap_uint(val & other.val); }
    ap_uint operator|(const ap_uint& other) const { return ap_uint(val | other.val); }
    ap_uint operator^(const ap_uint& other) const { return ap_uint(val ^ other.val); }
    ap_uint operator~() const { return ap_uint(~val); }
    ap_uint operator<<(int shift) const { return ap_uint(val << shift); }
    ap_uint operator>>(int shift) const { return ap_uint(val >> shift); }
    ap_uint operator<<(unsigned int shift) const { return ap_uint(val << shift); }
    ap_uint operator>>(unsigned int shift) const { return ap_uint(val >> shift); }

    // Mixed operators with int/unsigned
    ap_uint operator+(unsigned int other) const { return ap_uint(val + other); }
    ap_uint operator-(unsigned int other) const { return ap_uint(val - other); }
    ap_uint operator*(unsigned int other) const { return ap_uint(val * other); }
    ap_uint operator/(unsigned int other) const { return ap_uint(val / other); }
    ap_uint operator%(unsigned int other) const { return ap_uint(val % other); }
    ap_uint operator&(unsigned int other) const { return ap_uint(val & other); }
    ap_uint operator|(unsigned int other) const { return ap_uint(val | other); }
    ap_uint operator+(int other) const { return ap_uint(val + other); }
    ap_uint operator-(int other) const { return ap_uint(val - other); }
    ap_uint operator*(int other) const { return ap_uint(val * other); }

    // Compound assignment
    ap_uint& operator+=(const ap_uint& other) { val += other.val; return *this; }
    ap_uint& operator-=(const ap_uint& other) { val -= other.val; return *this; }
    ap_uint& operator*=(const ap_uint& other) { val *= other.val; return *this; }
    ap_uint& operator/=(const ap_uint& other) { val /= other.val; return *this; }
    ap_uint& operator%=(const ap_uint& other) { val %= other.val; return *this; }
    ap_uint& operator&=(const ap_uint& other) { val &= other.val; return *this; }
    ap_uint& operator|=(const ap_uint& other) { val |= other.val; return *this; }
    ap_uint& operator^=(const ap_uint& other) { val ^= other.val; return *this; }
    ap_uint& operator<<=(int shift) { val <<= shift; return *this; }
    ap_uint& operator>>=(int shift) { val >>= shift; return *this; }

    // Increment/decrement
    ap_uint& operator++() { ++val; return *this; }
    ap_uint operator++(int) { ap_uint tmp = *this; ++val; return tmp; }
    ap_uint& operator--() { --val; return *this; }
    ap_uint operator--(int) { ap_uint tmp = *this; --val; return tmp; }

    // Comparisons
    bool operator==(const ap_uint& other) const { return val == other.val; }
    bool operator!=(const ap_uint& other) const { return val != other.val; }
    bool operator<(const ap_uint& other) const { return val < other.val; }
    bool operator>(const ap_uint& other) const { return val > other.val; }
    bool operator<=(const ap_uint& other) const { return val <= other.val; }
    bool operator>=(const ap_uint& other) const { return val >= other.val; }
    template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    bool operator==(T other) const { return val == static_cast<uint64_t>(other); }
    template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    bool operator!=(T other) const { return val != static_cast<uint64_t>(other); }
    template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    bool operator<(T other) const { return val < static_cast<uint64_t>(other); }
    template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    bool operator>(T other) const { return val > static_cast<uint64_t>(other); }
    template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    bool operator<=(T other) const { return val <= static_cast<uint64_t>(other); }
    template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    bool operator>=(T other) const { return val >= static_cast<uint64_t>(other); }

    bool operator[](int i) const { return (val >> i) & 1; }

    // Range access — returns proxy that supports assignment
    ap_uint_range_ref<W> range(int hi, int lo) {
        return ap_uint_range_ref<W>(&val, hi, lo);
    }
    uint64_t range(int hi, int lo) const {
        return (val >> lo) & ((1ULL << (hi - lo + 1)) - 1);
    }
    ap_uint_range_ref<W> operator()(int hi, int lo) {
        return range(hi, lo);
    }
    uint64_t operator()(int hi, int lo) const {
        return range(hi, lo);
    }

    int length() const { return W; }
};

// Free functions for unsigned op ap_uint
template<int W>
ap_uint<W> operator+(unsigned int lhs, const ap_uint<W>& rhs) { return ap_uint<W>(lhs + rhs.val); }
template<int W>
ap_uint<W> operator-(unsigned int lhs, const ap_uint<W>& rhs) { return ap_uint<W>(lhs - rhs.val); }
template<int W>
ap_uint<W> operator*(unsigned int lhs, const ap_uint<W>& rhs) { return ap_uint<W>(lhs * rhs.val); }
template<int W>
bool operator==(unsigned int lhs, const ap_uint<W>& rhs) { return lhs == rhs.val; }
template<int W>
bool operator!=(unsigned int lhs, const ap_uint<W>& rhs) { return lhs != rhs.val; }
template<typename T, int W, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
bool operator<(T lhs, const ap_uint<W>& rhs) { return static_cast<long long>(lhs) < static_cast<long long>(rhs.val); }
template<typename T, int W, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
bool operator>(T lhs, const ap_uint<W>& rhs) { return static_cast<long long>(lhs) > static_cast<long long>(rhs.val); }
template<typename T, int W, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
bool operator<=(T lhs, const ap_uint<W>& rhs) { return static_cast<long long>(lhs) <= static_cast<long long>(rhs.val); }
template<typename T, int W, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
bool operator>=(T lhs, const ap_uint<W>& rhs) { return static_cast<long long>(lhs) >= static_cast<long long>(rhs.val); }

// Deferred implementations for range_ref assignment from ap_int/ap_uint
template<int W> template<int W2>
ap_range_ref<W>& ap_range_ref<W>::operator=(const ap_int<W2>& rhs) {
    return operator=(static_cast<int64_t>(rhs.val));
}
template<int W> template<int W2>
ap_range_ref<W>& ap_range_ref<W>::operator=(const ap_uint<W2>& rhs) {
    return operator=(static_cast<int64_t>(rhs.val));
}
template<int W> template<int W2>
ap_uint_range_ref<W>& ap_uint_range_ref<W>::operator=(const ap_int<W2>& rhs) {
    return operator=(static_cast<uint64_t>(rhs.val));
}
template<int W> template<int W2>
ap_uint_range_ref<W>& ap_uint_range_ref<W>::operator=(const ap_uint<W2>& rhs) {
    return operator=(static_cast<uint64_t>(rhs.val));
}

#endif // __AP_INT_H__
