#pragma once

// Xilinx HLS Vector Library stub
// This is a stub implementation for compilation testing only

#include <array>
#include <cstddef>

namespace hls {

template<typename T, std::size_t N>
class vector {
    std::array<T, N> data_;
public:
    using value_type = T;
    using size_type = std::size_t;
    static constexpr size_type size_v = N;
    
    vector() = default;
    vector(T val) { data_.fill(val); }
    
    T& operator[](size_type idx) { return data_[idx]; }
    const T& operator[](size_type idx) const { return data_[idx]; }
    
    static constexpr size_type size() { return N; }
};

} // namespace hls
