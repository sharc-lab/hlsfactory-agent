#ifndef AP_INT_H
#define AP_INT_H

#include <cstdint>
#include <type_traits>

// Forward declarations
template<int N> class ap_uint;
template<int N> class ap_int;

// Range reference class for assignment
template<int N>
class ap_range_ref {
public:
    using parent_t = ap_uint<N>;
    parent_t* parent;
    int hi, lo;
    
    ap_range_ref(parent_t* p, int h, int l) : parent(p), hi(h), lo(l) {}
    
    // Assignment from another ap_uint
    template<int M>
    ap_range_ref& operator=(const ap_uint<M>& val) {
        auto mask = ((1ULL << (hi - lo + 1)) - 1);
        parent->val = (parent->val & ~(mask << lo)) | ((val.val & mask) << lo);
        return *this;
    }
    
    // Assignment from integral types
    template<typename T>
    ap_range_ref& operator=(T val) {
        auto mask = ((1ULL << (hi - lo + 1)) - 1);
        parent->val = (parent->val & ~(mask << lo)) | ((static_cast<uint64_t>(val) & mask) << lo);
        return *this;
    }
    
    // Conversion to ap_uint
    template<int M>
    operator ap_uint<M>() const {
        return ap_uint<M>((parent->val >> lo) & ((1ULL << (hi - lo + 1)) - 1));
    }
    
    operator uint64_t() const {
        return (parent->val >> lo) & ((1ULL << (hi - lo + 1)) - 1);
    }
};

// Stub for ap_uint - Arbitrary precision unsigned integer
template<int N>
class ap_uint {
public:
    uint64_t val;
    
    ap_uint() : val(0) {}
    ap_uint(const ap_uint& other) : val(other.val) {}
    ap_uint(ap_uint&& other) : val(other.val) {}
    
    // Constructor from integral types
    template<typename T>
    ap_uint(T v) : val(static_cast<uint64_t>(v)) {}
    
    // Constructor from ap_int
    template<int M>
    ap_uint(const ap_int<M>& other) : val(static_cast<uint64_t>(other.val)) {}
    
    // Assignment operators
    ap_uint& operator=(const ap_uint& other) { val = other.val; return *this; }
    ap_uint& operator=(ap_uint&& other) { val = other.val; return *this; }
    
    template<typename T>
    ap_uint& operator=(T v) { val = static_cast<uint64_t>(v); return *this; }
    
    // Range access
    ap_range_ref<N> range(int hi, int lo) {
        return ap_range_ref<N>(this, hi, lo);
    }
    
    ap_uint range(int hi, int lo) const {
        return ap_uint((val >> lo) & ((1ULL << (hi - lo + 1)) - 1));
    }
    
    // Bitwise operators
    ap_uint operator&(const ap_uint& other) const { return ap_uint(val & other.val); }
    ap_uint operator|(const ap_uint& other) const { return ap_uint(val | other.val); }
    ap_uint operator^(const ap_uint& other) const { return ap_uint(val ^ other.val); }
    ap_uint operator~() const { return ap_uint(~val); }
    
    template<typename T>
    ap_uint operator&(T v) const { return ap_uint(val & static_cast<uint64_t>(v)); }
    template<typename T>
    ap_uint operator|(T v) const { return ap_uint(val | static_cast<uint64_t>(v)); }
    template<typename T>
    ap_uint operator^(T v) const { return ap_uint(val ^ static_cast<uint64_t>(v)); }
    
    ap_uint& operator&=(const ap_uint& other) { val &= other.val; return *this; }
    ap_uint& operator|=(const ap_uint& other) { val |= other.val; return *this; }
    ap_uint& operator^=(const ap_uint& other) { val ^= other.val; return *this; }
    
    template<typename T>
    ap_uint& operator&=(T v) { val &= static_cast<uint64_t>(v); return *this; }
    template<typename T>
    ap_uint& operator|=(T v) { val |= static_cast<uint64_t>(v); return *this; }
    template<typename T>
    ap_uint& operator^=(T v) { val ^= static_cast<uint64_t>(v); return *this; }
    
    // Shift operators
    ap_uint operator<<(int shift) const { return ap_uint(val << shift); }
    ap_uint operator>>(int shift) const { return ap_uint(val >> shift); }
    
    template<typename T>
    ap_uint operator<<(T shift) const { return ap_uint(val << shift); }
    template<typename T>
    ap_uint operator>>(T shift) const { return ap_uint(val >> shift); }
    
    ap_uint& operator<<=(int shift) { val <<= shift; return *this; }
    ap_uint& operator>>=(int shift) { val >>= shift; return *this; }
    
    // Arithmetic operators
    ap_uint operator+(const ap_uint& other) const { return ap_uint(val + other.val); }
    ap_uint operator-(const ap_uint& other) const { return ap_uint(val - other.val); }
    ap_uint operator*(const ap_uint& other) const { return ap_uint(val * other.val); }
    ap_uint operator/(const ap_uint& other) const { return ap_uint(val / other.val); }
    ap_uint operator%(const ap_uint& other) const { return ap_uint(val % other.val); }
    
    template<typename T>
    ap_uint operator+(T v) const { return ap_uint(val + static_cast<uint64_t>(v)); }
    template<typename T>
    ap_uint operator-(T v) const { return ap_uint(val - static_cast<uint64_t>(v)); }
    template<typename T>
    ap_uint operator*(T v) const { return ap_uint(val * static_cast<uint64_t>(v)); }
    template<typename T>
    ap_uint operator/(T v) const { return ap_uint(val / static_cast<uint64_t>(v)); }
    template<typename T>
    ap_uint operator%(T v) const { return ap_uint(val % static_cast<uint64_t>(v)); }
    
    ap_uint& operator+=(const ap_uint& other) { val += other.val; return *this; }
    ap_uint& operator-=(const ap_uint& other) { val -= other.val; return *this; }
    ap_uint& operator*=(const ap_uint& other) { val *= other.val; return *this; }
    ap_uint& operator/=(const ap_uint& other) { val /= other.val; return *this; }
    ap_uint& operator%=(const ap_uint& other) { val %= other.val; return *this; }
    
    // Comparison operators
    bool operator==(const ap_uint& other) const { return val == other.val; }
    bool operator!=(const ap_uint& other) const { return val != other.val; }
    bool operator<(const ap_uint& other) const { return val < other.val; }
    bool operator>(const ap_uint& other) const { return val > other.val; }
    bool operator<=(const ap_uint& other) const { return val <= other.val; }
    bool operator>=(const ap_uint& other) const { return val >= other.val; }
    
    template<typename T>
    bool operator==(T v) const { return val == static_cast<uint64_t>(v); }
    template<typename T>
    bool operator!=(T v) const { return val != static_cast<uint64_t>(v); }
    template<typename T>
    bool operator<(T v) const { return val < static_cast<uint64_t>(v); }
    template<typename T>
    bool operator>(T v) const { return val > static_cast<uint64_t>(v); }
    template<typename T>
    bool operator<=(T v) const { return val <= static_cast<uint64_t>(v); }
    template<typename T>
    bool operator>=(T v) const { return val >= static_cast<uint64_t>(v); }
    
    // Increment/Decrement
    ap_uint& operator++() { ++val; return *this; }
    ap_uint operator++(int) { ap_uint tmp(*this); ++val; return tmp; }
    ap_uint& operator--() { --val; return *this; }
    ap_uint operator--(int) { ap_uint tmp(*this); --val; return tmp; }
    
    // Unary operators
    ap_uint operator+() const { return *this; }
    ap_uint operator-() const { return ap_uint(-static_cast<int64_t>(val)); }
    
    // Boolean conversion
    operator bool() const { return val != 0; }
    
    // Explicit conversion
    operator uint8_t() const { return static_cast<uint8_t>(val); }
    operator uint16_t() const { return static_cast<uint16_t>(val); }
    operator uint32_t() const { return static_cast<uint32_t>(val); }
    operator uint64_t() const { return val; }
    operator int() const { return static_cast<int>(val); }
    operator long() const { return static_cast<long>(val); }
    operator long long() const { return static_cast<long long>(val); }
    
    // Bit access
    bool test(int bit) const { return (val >> bit) & 1; }
    void set(int bit) { val |= (1ULL << bit); }
    void clear(int bit) { val &= ~(1ULL << bit); }
};

// Range reference for ap_int
template<int N>
class ap_int_range_ref {
public:
    using parent_t = ap_int<N>;
    parent_t* parent;
    int hi, lo;
    
    ap_int_range_ref(parent_t* p, int h, int l) : parent(p), hi(h), lo(l) {}
    
    template<typename T>
    ap_int_range_ref& operator=(T val) {
        auto mask = ((1LL << (hi - lo + 1)) - 1);
        parent->val = (parent->val & ~(mask << lo)) | ((static_cast<int64_t>(val) & mask) << lo);
        return *this;
    }
    
    operator int64_t() const {
        return (parent->val >> lo) & ((1LL << (hi - lo + 1)) - 1);
    }
};

// Stub for ap_int - Arbitrary precision signed integer
template<int N>
class ap_int {
public:
    int64_t val;
    
    ap_int() : val(0) {}
    ap_int(const ap_int& other) : val(other.val) {}
    ap_int(ap_int&& other) : val(other.val) {}
    
    template<typename T>
    ap_int(T v) : val(static_cast<int64_t>(v)) {}
    
    ap_int& operator=(const ap_int& other) { val = other.val; return *this; }
    ap_int& operator=(ap_int&& other) { val = other.val; return *this; }
    
    template<typename T>
    ap_int& operator=(T v) { val = static_cast<int64_t>(v); return *this; }
    
    ap_int_range_ref<N> range(int hi, int lo) {
        return ap_int_range_ref<N>(this, hi, lo);
    }
    
    ap_uint<N> range(int hi, int lo) const {
        return ap_uint<N>((val >> lo) & ((1LL << (hi - lo + 1)) - 1));
    }
    
    // Arithmetic operators
    ap_int operator+(const ap_int& other) const { return ap_int(val + other.val); }
    ap_int operator-(const ap_int& other) const { return ap_int(val - other.val); }
    ap_int operator*(const ap_int& other) const { return ap_int(val * other.val); }
    ap_int operator/(const ap_int& other) const { return ap_int(val / other.val); }
    ap_int operator%(const ap_int& other) const { return ap_int(val % other.val); }
    
    // Bitwise operators
    ap_int operator&(const ap_int& other) const { return ap_int(val & other.val); }
    ap_int operator|(const ap_int& other) const { return ap_int(val | other.val); }
    ap_int operator^(const ap_int& other) const { return ap_int(val ^ other.val); }
    ap_int operator~() const { return ap_int(~val); }
    
    ap_int operator<<(int shift) const { return ap_int(val << shift); }
    ap_int operator>>(int shift) const { return ap_int(val >> shift); }
    
    // Comparison operators
    bool operator==(const ap_int& other) const { return val == other.val; }
    bool operator!=(const ap_int& other) const { return val != other.val; }
    bool operator<(const ap_int& other) const { return val < other.val; }
    bool operator>(const ap_int& other) const { return val > other.val; }
    bool operator<=(const ap_int& other) const { return val <= other.val; }
    bool operator>=(const ap_int& other) const { return val >= other.val; }
    
    // Unary operators
    ap_int operator+() const { return *this; }
    ap_int operator-() const { return ap_int(-val); }
    
    operator bool() const { return val != 0; }
    
    operator int() const { return static_cast<int>(val); }
    operator long() const { return static_cast<long>(val); }
    operator long long() const { return val; }
};

#endif // AP_INT_H
