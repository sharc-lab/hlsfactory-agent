#pragma once

#include <iostream>
#include <sstream>
#include <string>

// Simple to_string for int
inline std::string to_string(int v) { std::ostringstream oss; oss << v; return oss.str(); }

// Stub data type definitions
using Data_t = float;
using half = float;  // placeholder for half-precision

// Memory bus widths in bytes (example values)
constexpr int kMemoryWidthBytesK = 32;
constexpr int kMemoryWidthBytesM = 32;
constexpr int kMemoryWidthBytesN = 32;

// Transpose width in bytes
constexpr int kTransposeWidthBytes = 32;

// Compute tile sizes
constexpr int kComputeTileSizeN = 8;
constexpr int kComputeTileSizeM = 8;

// Tile dimensions (example values)
constexpr int kInnerTileSizeN = 32;
constexpr int kInnerTileSizeM = 32;
constexpr int kOuterTileSizeN = 128;
constexpr int kOuterTileSizeM = 128;

// Problem sizes (example values)
constexpr int kSizeN = 64;
constexpr int kSizeM = 64;
constexpr int kSizeK = 64;

// Dummy operator implementations for compilation
namespace hlslib {
    namespace op {
        template<typename T>
        struct DummyOp {
            static T Apply(const T& a, const T& b) { return a + b; }
            static T identity() { return T{}; }
        };
    }
    // Simple compile‑time log2 stub
    constexpr int ConstLog2(int) { return 0; }
}

// Map and Reduce operator aliases
using OperatorMap = hlslib::op::DummyOp<Data_t>;
using OperatorReduce = hlslib::op::DummyOp<Data_t>;

// Minimal ap_uint stub with flexible comparison operators
template<int W>
struct ap_uint {
    unsigned int value;
    ap_uint() : value(0) {}
    ap_uint(unsigned int v) : value(v) {}

    // Assignment from unsigned int
    ap_uint& operator=(unsigned int v) { value = v; return *this; }

    // Increment
    ap_uint& operator++() { ++value; return *this; }

    // Conversion to unsigned int
    operator unsigned int() const { return value; }

    // Equality/inequality with another ap_uint
    bool operator==(const ap_uint& other) const { return value == other.value; }
    bool operator!=(const ap_uint& other) const { return value != other.value; }

    // Equality/inequality with any integral type
    template<typename T,
             typename = typename std::enable_if<std::is_integral<T>::value>::type>
    bool operator==(T rhs) const { return value == static_cast<unsigned int>(rhs); }

    template<typename T,
             typename = typename std::enable_if<std::is_integral<T>::value>::type>
    bool operator!=(T rhs) const { return value != static_cast<unsigned int>(rhs); }
};

