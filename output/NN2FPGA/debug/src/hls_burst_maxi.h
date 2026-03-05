#pragma once

// Xilinx HLS Burst Maxi Library stub
// This is a stub implementation for compilation testing only

#include <cstddef>
#include <cstdint>

namespace hls {

// Burst memory interface stub
template<typename T>
class burst_maxi {
    T* ptr_;
public:
    burst_maxi() : ptr_(nullptr) {}
    burst_maxi(T* ptr) : ptr_(ptr) {}
    
    T* get() const { return ptr_; }
    void write(T* ptr) { ptr_ = ptr; }
    
    // Burst read
    void read(T* dest, size_t offset, size_t num_elements) {}
    
    // Burst write
    void write(const T* src, size_t offset, size_t num_elements) {}
    
    T& operator[](size_t idx) { return ptr_[idx]; }
    const T& operator[](size_t idx) const { return ptr_[idx]; }
};

// For compatibility
template<typename T>
using hls_burst_maxi = burst_maxi<T>;

} // namespace hls

// Compatibility macros
#define HLS_BURST_MAXI
