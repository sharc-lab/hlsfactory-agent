#ifndef TAPA_H
#define TAPA_H

#include <cstdint>
#include <queue>
#include <vector>
#include <string>

namespace tapa {

// TAPA Stream stub
template<typename T, int DEPTH = 0>
class stream {
public:
    std::queue<T> data;
    
    bool empty() { return data.empty(); }
    bool full() { return false; }
    T read() { T val = data.front(); data.pop(); return val; }
    void write(T val) { data.push(val); }
    T read(bool* success) { *success = !empty(); return read(); }
    bool try_write(T val) { data.push(val); return true; }
};

// TAPA istream/ostream typedefs
template<typename T, int DEPTH = 0>
using istream = stream<T, DEPTH>;

template<typename T, int DEPTH = 0>
using ostream = stream<T, DEPTH>;

// TAPA streams (array of streams)
template<typename T, int N, int DEPTH = 0>
class streams {
public:
    stream<T, DEPTH> s[N];
    
    stream<T, DEPTH>& operator[](int idx) { return s[idx]; }
};

// TAPA istreams/ostreams typedefs
template<typename T, int N, int DEPTH = 0>
using istreams = streams<T, N, DEPTH>;

template<typename T, int N, int DEPTH = 0>
using ostreams = streams<T, N, DEPTH>;

// TAPA mmap stub
template<typename T>
class mmap {
public:
    T* ptr;
    
    mmap(T* p = nullptr) : ptr(p) {}
    
    T& operator[](size_t idx) { return ptr[idx]; }
};

// TAPA async_mmap stub
template<typename T>
class async_mmap {
public:
    T* ptr;
    stream<size_t> read_addr;
    stream<T> read_data;
    stream<size_t> write_addr;
    stream<T> write_data;
    stream<uint8_t> write_resp;
    
    async_mmap(T* p = nullptr) : ptr(p) {}
    
    bool try_write(size_t addr) { read_addr.write(addr); return true; }
    T read(bool* success) { return read_data.read(success); }
};

// TAPA task stub
class task {
public:
    task() {}
    
    template<typename Func, typename... Args>
    task& invoke(Func&& f, Args&&... args) {
        return *this;
    }
    
    template<typename Func, typename... Args>
    task& invoke(task t, Func&& f, Args&&... args) {
        return *this;
    }
    
    template<int N, typename Func, typename... Args>
    task& invoke(Func&& f, Args&&... args) {
        return *this;
    }
    
    template<int N, typename Func, typename... Args>
    task& invoke(task t, Func&& f, Args&&... args) {
        return *this;
    }
};

// TAPA detach tag
struct detach_t {};
static constexpr detach_t detach;

// TAPA join tag
struct join_t {};
static constexpr join_t join;

} // namespace tapa

using namespace tapa;

#endif // TAPA_H
