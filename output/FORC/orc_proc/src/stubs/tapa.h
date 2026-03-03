/*
 * Stub header for TAPA framework
 * For compilation testing with clang++ only - NOT for HLS synthesis
 */

#ifndef __TAPA_H__
#define __TAPA_H__

#include <cstdint>
#include <queue>
#include <string>
#include <vector>
#include <memory>
#include <functional>

// Include ap_int stub
#include "ap_int.h"

namespace tapa {

// Memory-mapped interface stub
template<typename T>
class mmap {
public:
    T* ptr;
    mmap(T* p = nullptr) : ptr(p) {}
    
    T& operator[](size_t idx) { return ptr[idx]; }
    const T& operator[](size_t idx) const { return ptr[idx]; }
};

// Async mmap interface stub
template<typename T>
class async_mmap {
public:
    struct write_addr_port {
        bool full() const { return false; }
        template<typename U>
        bool try_write(U) { return true; }
    } write_addr;
    
    struct write_data_port {
        bool full() const { return false; }
        template<typename U>
        bool try_write(U) { return true; }
    } write_data;
    
    struct write_resp_port {
        bool empty() const { return true; }
        template<typename U>
        U read(U*) { return U(); }
    } write_resp;
    
    struct read_addr_port {
        bool full() const { return false; }
        template<typename U>
        bool try_write(U) { return true; }
    } read_addr;
    
    struct read_data_port {
        bool empty() const { return true; }
        template<typename U>
        U read(U*) { return U(); }
    } read_data;
};

// Stream interface stub
template<typename T, int DEPTH = 0>
class stream {
private:
    std::queue<T> data;
    std::string name;

public:
    stream() : name("") {}
    stream(const char* n) : name(n) {}
    
    void write(const T& val) { data.push(val); }
    T read() { T val = data.front(); data.pop(); return val; }
    bool empty() const { return data.empty(); }
    bool full() const { return false; }
    size_t size() const { return data.size(); }
    
    // Non-blocking operations
    bool try_read(T& val) {
        if (data.empty()) return false;
        val = data.front(); data.pop(); return true;
    }
    bool try_write(const T& val) { data.push(val); return true; }
    
    void operator<<(const T& val) { write(val); }
    void operator>>(T& val) { val = read(); }
};

// Multi-streams stub
template<typename T, int N, int DEPTH = 0>
class streams {
private:
    std::vector<stream<T, DEPTH>> data_streams;
public:
    streams() : data_streams(N) {}
    streams(const char*) : data_streams(N) {}
    
    stream<T, DEPTH>& operator[](size_t idx) { return data_streams[idx % N]; }
};

// istream/ostream type aliases
template<typename T, int DEPTH = 0>
using istream = stream<T, DEPTH>;

template<typename T, int DEPTH = 0>
using ostream = stream<T, DEPTH>;

template<typename T, int N, int DEPTH = 0>
using istreams = streams<T, N, DEPTH>;

template<typename T, int N, int DEPTH = 0>
using ostreams = streams<T, N, DEPTH>;

// Task invocation stub
class task {
public:
    template<typename Func, typename... Args>
    task& invoke(Func&& f, Args&&... args) { return *this; }
    
    template<typename... Args>
    task& invoke(Args&&... args) { return *this; }
};

// Detach and join modifiers
struct detach_t {};
struct join_t {};
constexpr detach_t detach;
constexpr join_t join;

// Allocator stub
template<typename T>
class aligned_allocator : public std::allocator<T> {};

} // namespace tapa

#endif // __TAPA_H__
