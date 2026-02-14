// TAPA HLS Stub Header
// This is a stub implementation of the TAPA library for HLS synthesis

#ifndef TAPA_H
#define TAPA_H

#include <hls_stream.h>
#include <ap_int.h>

namespace tapa {

// Vector type definition
template<typename T, int N>
using vec_t = T[N];

// Memory map interface (simplified for HLS)
template<typename T>
class mmap {
public:
    T* ptr;
    mmap() : ptr(nullptr) {}
    mmap(T* p) : ptr(p) {}
    
    T& operator[](size_t idx) {
        return ptr[idx];
    }
};

// Async memory map interface
template<typename T>
class async_mmap {
public:
    T* ptr;
    hls::stream<size_t> read_addr;
    hls::stream<T> read_data;
    hls::stream<size_t> write_addr;
    hls::stream<T> write_data;
    hls::stream<ap_uint<8>> write_resp;
    
    async_mmap() : ptr(nullptr) {}
    async_mmap(T* p) : ptr(p) {}
    
    bool full() { return false; }
    bool empty() { return read_data.empty(); }
};

// Stream interface
template<typename T>
using stream = hls::stream<T>;

// Input stream
template<typename T>
using istream = hls::stream<T>;

// Output stream
template<typename T>
using ostream = hls::stream<T>;

// Multiple streams
template<typename T, int N>
using istreams = hls::stream<T>[N];

template<typename T, int N>
using ostreams = hls::stream<T>[N];

template<typename T, int N>
using streams = hls::stream<T>[N];

// Task class for parallel execution (stub)
class task {
public:
    template<typename Func, typename... Args>
    task& invoke(Func&& f, Args&&... args) {
        // In HLS, this would be synthesized as separate processes
        return *this;
    }
    
    template<typename Tag, typename Func, typename... Args>
    task& invoke(Func&& f, Args&&... args) {
        return *this;
    }
};

// Join tag for parallel tasks
struct join {};

// Detach tag for detached tasks
struct detach {};

} // namespace tapa

#endif // TAPA_H
