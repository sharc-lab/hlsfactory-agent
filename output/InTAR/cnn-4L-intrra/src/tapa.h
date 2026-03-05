/* Stub header for TAPA framework */
#ifndef TAPA_H
#define TAPA_H

#include <cstdint>
#include <utility>
#include <type_traits>

// Forward declarations
namespace tapa {

template <typename T, int N>
struct vec_t {
    T data[N];
    
    vec_t() = default;
    
    T& operator[](int idx) { return data[idx]; }
    const T& operator[](int idx) const { return data[idx]; }
};

// Streams
template <typename T>
class ostream {
public:
    bool full() const { return false; }
    void write(const T& val) {}
    bool try_write(const T& val) { return true; }
    T& front() { static T val; return val; }
};

template <typename T>
class istream {
public:
    bool empty() const { return false; }
    T read() { return T(); }
    bool try_read(T& val) { return true; }
    void pop() {}
    T& front() { static T val; return val; }
};

// Multi-streams
template <typename T, int N>
class ostreams {
public:
    bool full(int idx) const { return false; }
    void write(int idx, const T& val) {}
    bool try_write(int idx, const T& val) { return true; }
    ostream<T>& operator[](int idx) { static ostream<T> s; return s; }
};

template <typename T, int N>
class istreams {
public:
    bool empty(int idx) const { return false; }
    T read(int idx) { return T(); }
    bool try_read(int idx, T& val) { return true; }
    void pop(int idx) {}
    T& front(int idx) { static T val; return val; }
    istream<T>& operator[](int idx) { static istream<T> s; return s; }
};

// async_mmap
template <typename T>
class async_mmap {
public:
    class addr_stream {
    public:
        bool full() const { return false; }
        void write(int val) {}
        bool try_write(int val) { return true; }
    };
    
    class data_stream {
    public:
        bool full() const { return false; }
        void write(const T& val) {}
        bool try_write(const T& val) { return true; }
        bool try_read(T& val) { return true; }
    };
    
    class resp_stream {
    public:
        bool empty() const { return false; }
        int read(bool& success) { success = true; return 0; }
    };
    
    addr_stream read_addr;
    addr_stream write_addr;
    data_stream read_data;
    data_stream write_data;
    resp_stream write_resp;
};

} // namespace tapa

#ifndef __SYNTHESIS__
#define __SYNTHESIS__
#endif

#endif // TAPA_H
