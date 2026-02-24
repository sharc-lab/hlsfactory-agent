// Stub header for TAPA framework
#ifndef TAPA_H
#define TAPA_H

#include "ap_int.h"
#include "hls_stream.h"

namespace tapa {

// Stream types
template<typename T, int DEPTH = 2>
using stream = hls::stream<T>;

template<typename T, int N, int DEPTH = 2>
using streams = hls::stream<T>[N];

// Memory-mapped types
template<typename T>
using mmap = T*;

template<typename T>
class async_mmap {
public:
    hls::stream<ap_uint<64>> read_addr;
    hls::stream<T> read_data;
    hls::stream<ap_uint<64>> write_addr;
    hls::stream<T> write_data;
    hls::stream<ap_uint<8>> write_resp;
    
    T& operator[](size_t idx) {
        static T dummy;
        return dummy;
    }
};

// Istream/Ostream aliases
template<typename T>
using istream = hls::stream<T>;

template<typename T>
using ostream = hls::stream<T>;

template<typename T, int N>
using istreams = hls::stream<T>[N];

template<typename T, int N>
using ostreams = hls::stream<T>[N];

// Task class
class task {
public:
    template<typename Func, typename... Args>
    task& invoke(Func&& f, Args&&... args) {
        return *this;
    }
    
    template<typename Func, int N, typename... Args>
    task& invoke(Func&& f, Args&&... args) {
        return *this;
    }
    
    template<typename Mode, typename Func, typename... Args>
    task& invoke(Func&& f, Args&&... args) {
        return *this;
    }
    
    template<typename Mode, int N, typename Func, typename... Args>
    task& invoke(Func&& f, Args&&... args) {
        return *this;
    }
};

// Execution modes
struct detach {};
struct join {};

} // namespace tapa

#endif
