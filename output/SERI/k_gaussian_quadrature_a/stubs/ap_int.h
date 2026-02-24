/*
 * Improved stub header for Xilinx ap_int.h
 * For compilation testing with clang++ only - NOT for HLS synthesis
 */

#ifndef __AP_INT_H__
#define __AP_INT_H__

#include <cstdint>
#include <limits>
#include <cstring>
#include <vector>
#include <algorithm>

// Use a byte array for arbitrary precision
template<int W>
class ap_uint {
public:
    static constexpr int bit_width = W;
    static constexpr int byte_width = (W + 7) / 8;
    
    uint8_t data[byte_width];
    
    ap_uint() { std::memset(data, 0, byte_width); }
    
    ap_uint(unsigned int v) { 
        std::memset(data, 0, byte_width);
        for (int i = 0; i < std::min((int)sizeof(v), byte_width); ++i) {
            data[i] = (v >> (i * 8)) & 0xFF;
        }
    }
    
    ap_uint(unsigned long long v) { 
        std::memset(data, 0, byte_width);
        for (int i = 0; i < std::min((int)sizeof(v), byte_width); ++i) {
            data[i] = (v >> (i * 8)) & 0xFF;
        }
    }
    
    template<int W2>
    ap_uint(const ap_uint<W2>& other) {
        std::memset(data, 0, byte_width);
        int min_bytes = std::min(byte_width, other.byte_width);
        std::memcpy(data, other.data, min_bytes);
    }
    
    // Constructor from two ap_uint (concatenation): (hi, lo)
    template<int WH, int WL>
    ap_uint(const ap_uint<WH>& hi, const ap_uint<WL>& lo) {
        std::memset(data, 0, byte_width);
        // Copy lo part
        int lo_bytes = std::min((WL + 7) / 8, byte_width);
        std::memcpy(data, lo.data, lo_bytes);
        // Copy hi part
        int hi_bytes = std::min((WH + 7) / 8, byte_width - lo_bytes);
        if (hi_bytes > 0) {
            std::memcpy(data + lo_bytes, hi.data, hi_bytes);
        }
    }
    
    // Assignment operators
    ap_uint& operator=(unsigned int v) {
        std::memset(data, 0, byte_width);
        for (int i = 0; i < std::min((int)sizeof(v), byte_width); ++i) {
            data[i] = (v >> (i * 8)) & 0xFF;
        }
        return *this;
    }
    
    template<int W2>
    ap_uint& operator=(const ap_uint<W2>& other) {
        std::memset(data, 0, byte_width);
        int min_bytes = std::min(byte_width, other.byte_width);
        std::memcpy(data, other.data, min_bytes);
        return *this;
    }
    
    // Range access
    template<int H, int L>
    ap_uint<H-L+1> range() const {
        ap_uint<H-L+1> result;
        int start_byte = L / 8;
        int start_bit = L % 8;
        int end_byte = H / 8;
        
        uint64_t temp = 0;
        for (int i = start_byte; i <= std::min(end_byte, byte_width - 1); ++i) {
            temp |= (uint64_t)data[i] << ((i - start_byte) * 8);
        }
        temp >>= start_bit;
        temp &= (H - L + 1 == 64) ? ~0ULL : ((1ULL << (H - L + 1)) - 1);
        
        for (int i = 0; i < result.byte_width; ++i) {
            result.data[i] = (temp >> (i * 8)) & 0xFF;
        }
        return result;
    }
    
    ap_uint range(int hi, int lo) const {
        int width = hi - lo + 1;
        ap_uint<1> result;  // Will be resized
        // Simplified - returns based on width needed
        if (width <= 64) {
            uint64_t temp = 0;
            for (int i = lo / 8; i <= std::min(hi / 8, byte_width - 1); ++i) {
                temp |= (uint64_t)data[i] << ((i - lo / 8) * 8);
            }
            temp >>= (lo % 8);
            temp &= (width == 64) ? ~0ULL : ((1ULL << width) - 1);
            // Store in a way that can be extracted
            (void)temp;
        }
        return ap_uint<1>();
    }
    
    // Bit access
    bool operator[](int i) const {
        int byte_idx = i / 8;
        int bit_idx = i % 8;
        if (byte_idx < byte_width) {
            return (data[byte_idx] >> bit_idx) & 1;
        }
        return false;
    }
    
    // Arithmetic operators
    ap_uint operator+(const ap_uint& other) const {
        ap_uint result;
        uint64_t carry = 0;
        for (int i = 0; i < std::min(byte_width, (int)sizeof(carry)); ++i) {
            uint64_t sum = (uint64_t)data[i] + other.data[i] + carry;
            result.data[i] = sum & 0xFF;
            carry = sum >> 8;
        }
        return result;
    }
    
    ap_uint operator-(const ap_uint& other) const {
        ap_uint result;
        int64_t borrow = 0;
        for (int i = 0; i < byte_width; ++i) {
            int64_t diff = (int64_t)data[i] - other.data[i] - borrow;
            if (diff < 0) {
                diff += 256;
                borrow = 1;
            } else {
                borrow = 0;
            }
            result.data[i] = diff & 0xFF;
        }
        return result;
    }
    
    ap_uint operator*(const ap_uint& other) const {
        // Simplified multiplication
        uint64_t a = 0, b = 0;
        for (int i = std::min(byte_width, 8) - 1; i >= 0; --i) {
            a = (a << 8) | data[i];
            b = (b << 8) | other.data[i];
        }
        ap_uint result;
        uint64_t prod = a * b;
        for (int i = 0; i < std::min(byte_width, 8); ++i) {
            result.data[i] = (prod >> (i * 8)) & 0xFF;
        }
        return result;
    }
    
    ap_uint operator/(const ap_uint& other) const {
        uint64_t a = 0, b = 0;
        for (int i = std::min(byte_width, 8) - 1; i >= 0; --i) {
            a = (a << 8) | data[i];
            b = (b << 8) | other.data[i];
        }
        ap_uint result;
        uint64_t quot = (b != 0) ? a / b : 0;
        for (int i = 0; i < std::min(byte_width, 8); ++i) {
            result.data[i] = (quot >> (i * 8)) & 0xFF;
        }
        return result;
    }
    
    ap_uint operator%(const ap_uint& other) const {
        uint64_t a = 0, b = 0;
        for (int i = std::min(byte_width, 8) - 1; i >= 0; --i) {
            a = (a << 8) | data[i];
            b = (b << 8) | other.data[i];
        }
        ap_uint result;
        uint64_t rem = (b != 0) ? a % b : 0;
        for (int i = 0; i < std::min(byte_width, 8); ++i) {
            result.data[i] = (rem >> (i * 8)) & 0xFF;
        }
        return result;
    }
    
    ap_uint operator&(const ap_uint& other) const {
        ap_uint result;
        for (int i = 0; i < byte_width; ++i) {
            result.data[i] = data[i] & other.data[i];
        }
        return result;
    }
    
    ap_uint operator|(const ap_uint& other) const {
        ap_uint result;
        for (int i = 0; i < byte_width; ++i) {
            result.data[i] = data[i] | other.data[i];
        }
        return result;
    }
    
    ap_uint operator^(const ap_uint& other) const {
        ap_uint result;
        for (int i = 0; i < byte_width; ++i) {
            result.data[i] = data[i] ^ other.data[i];
        }
        return result;
    }
    
    ap_uint operator<<(int shift) const {
        ap_uint result;
        int byte_shift = shift / 8;
        int bit_shift = shift % 8;
        
        for (int i = byte_width - 1; i >= 0; --i) {
            int src_idx = i - byte_shift;
            if (src_idx >= 0 && src_idx < byte_width) {
                result.data[i] = data[src_idx] << bit_shift;
                if (src_idx > 0 && bit_shift > 0) {
                    result.data[i] |= data[src_idx - 1] >> (8 - bit_shift);
                }
            }
        }
        return result;
    }
    
    ap_uint operator>>(int shift) const {
        ap_uint result;
        int byte_shift = shift / 8;
        int bit_shift = shift % 8;
        
        for (int i = 0; i < byte_width; ++i) {
            int src_idx = i + byte_shift;
            if (src_idx >= 0 && src_idx < byte_width) {
                result.data[i] = data[src_idx] >> bit_shift;
                if (src_idx + 1 < byte_width && bit_shift > 0) {
                    result.data[i] |= data[src_idx + 1] << (8 - bit_shift);
                }
            }
        }
        return result;
    }
    
    // Compound assignment
    ap_uint& operator+=(const ap_uint& other) { *this = *this + other; return *this; }
    ap_uint& operator-=(const ap_uint& other) { *this = *this - other; return *this; }
    ap_uint& operator&=(const ap_uint& other) { *this = *this & other; return *this; }
    ap_uint& operator|=(const ap_uint& other) { *this = *this | other; return *this; }
    
    // Increment/Decrement
    ap_uint& operator++() { *this = *this + ap_uint(1); return *this; }
    ap_uint operator++(int) { ap_uint tmp = *this; ++(*this); return tmp; }
    
    // Comparison operators
    bool operator==(const ap_uint& other) const {
        for (int i = byte_width - 1; i >= 0; --i) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }
    
    bool operator!=(const ap_uint& other) const { return !(*this == other); }
    
    bool operator<(const ap_uint& other) const {
        for (int i = byte_width - 1; i >= 0; --i) {
            if (data[i] < other.data[i]) return true;
            if (data[i] > other.data[i]) return false;
        }
        return false;
    }
    
    bool operator>(const ap_uint& other) const { return other < *this; }
    bool operator<=(const ap_uint& other) const { return !(other < *this); }
    bool operator>=(const ap_uint& other) const { return !(*this < other); }
    
    // Conversion operators
    operator unsigned int() const {
        unsigned int result = 0;
        for (int i = std::min(byte_width, (int)sizeof(result)) - 1; i >= 0; --i) {
            result = (result << 8) | data[i];
        }
        return result;
    }
    
    operator unsigned long long() const {
        unsigned long long result = 0;
        for (int i = std::min(byte_width, (int)sizeof(result)) - 1; i >= 0; --i) {
            result = (result << 8) | data[i];
        }
        return result;
    }
    
    operator int() const { return static_cast<int>(operator unsigned int()); }
    operator long long() const { return static_cast<long long>(operator unsigned long long()); }
    operator float() const { return static_cast<float>(operator unsigned long long()); }
    operator double() const { return static_cast<double>(operator unsigned long long()); }
};

// ap_int - signed version
template<int W>
class ap_int {
public:
    static constexpr int bit_width = W;
    static constexpr int byte_width = (W + 7) / 8;
    
    uint8_t data[byte_width];
    
    ap_int() { std::memset(data, 0, byte_width); }
    
    ap_int(int v) {
        std::memset(data, 0, byte_width);
        for (int i = 0; i < std::min((int)sizeof(v), byte_width); ++i) {
            data[i] = (v >> (i * 8)) & 0xFF;
        }
    }
    
    ap_int(long long v) {
        std::memset(data, 0, byte_width);
        for (int i = 0; i < std::min((int)sizeof(v), byte_width); ++i) {
            data[i] = (v >> (i * 8)) & 0xFF;
        }
    }
    
    template<int W2>
    ap_int(const ap_int<W2>& other) {
        std::memset(data, 0, byte_width);
        int min_bytes = std::min(byte_width, other.byte_width);
        std::memcpy(data, other.data, min_bytes);
    }
    
    ap_int& operator=(int v) {
        std::memset(data, 0, byte_width);
        for (int i = 0; i < std::min((int)sizeof(v), byte_width); ++i) {
            data[i] = (v >> (i * 8)) & 0xFF;
        }
        return *this;
    }
    
    // Bit access
    bool operator[](int i) const {
        int byte_idx = i / 8;
        int bit_idx = i % 8;
        if (byte_idx < byte_width) {
            return (data[byte_idx] >> bit_idx) & 1;
        }
        return false;
    }
    
    // Basic arithmetic
    ap_int operator+(const ap_int& other) const {
        ap_int result;
        uint64_t carry = 0;
        for (int i = 0; i < std::min(byte_width, (int)sizeof(carry)); ++i) {
            uint64_t sum = (uint64_t)data[i] + other.data[i] + carry;
            result.data[i] = sum & 0xFF;
            carry = sum >> 8;
        }
        return result;
    }
    
    ap_int operator-(const ap_int& other) const {
        ap_int result;
        int64_t borrow = 0;
        for (int i = 0; i < byte_width; ++i) {
            int64_t diff = (int64_t)data[i] - other.data[i] - borrow;
            if (diff < 0) {
                diff += 256;
                borrow = 1;
            } else {
                borrow = 0;
            }
            result.data[i] = diff & 0xFF;
        }
        return result;
    }
    
    ap_int operator*(const ap_int& other) const {
        int64_t a = 0, b = 0;
        for (int i = std::min(byte_width, 8) - 1; i >= 0; --i) {
            a = (a << 8) | data[i];
            b = (b << 8) | other.data[i];
        }
        ap_int result;
        int64_t prod = a * b;
        for (int i = 0; i < std::min(byte_width, 8); ++i) {
            result.data[i] = (prod >> (i * 8)) & 0xFF;
        }
        return result;
    }
    
    ap_int& operator++() { *this = *this + ap_int(1); return *this; }
    
    // Comparison
    bool operator==(const ap_int& other) const {
        for (int i = byte_width - 1; i >= 0; --i) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }
    
    bool operator!=(const ap_int& other) const { return !(*this == other); }
    bool operator<(const ap_int& other) const {
        for (int i = byte_width - 1; i >= 0; --i) {
            if (data[i] < other.data[i]) return true;
            if (data[i] > other.data[i]) return false;
        }
        return false;
    }
    
    // Conversion
    operator int() const {
        int result = 0;
        for (int i = std::min(byte_width, (int)sizeof(result)) - 1; i >= 0; --i) {
            result = (result << 8) | data[i];
        }
        return result;
    }
    
    operator long long() const {
        long long result = 0;
        for (int i = std::min(byte_width, (int)sizeof(result)) - 1; i >= 0; --i) {
            result = (result << 8) | data[i];
        }
        return result;
    }
    
    operator float() const { return static_cast<float>(operator long long()); }
    operator double() const { return static_cast<double>(operator long long()); }
};

// Concatenation operator: (hi, lo) -> creates ap_uint<WH+WL>
template<int WH, int WL>
ap_uint<WH + WL> operator,(const ap_uint<WH>& hi, const ap_uint<WL>& lo) {
    return ap_uint<WH + WL>(hi, lo);
}

#endif // __AP_INT_H__
