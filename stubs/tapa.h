/*
 * Stub header for TAPA (Task-Parallel HLS)
 * For compilation testing with clang++ only - NOT for HLS synthesis
 *
 * Provides minimal type definitions to allow TAPA-based HLS code
 * to compile with standard compilers.
 */

#ifndef __TAPA_H__
#define __TAPA_H__

#include <cstdint>
#include <cstddef>
#include <queue>
#include <vector>
#include <functional>

// Include common HLS types that TAPA designs often use
#include "ap_int.h"
#include "ap_fixed.h"

namespace tapa {

// ---- Memory-mapped interfaces ----

template<typename T>
class mmap {
    T* ptr_;
    size_t size_;
public:
    mmap() : ptr_(nullptr), size_(0) {}
    mmap(T* p, size_t n) : ptr_(p), size_(n) {}

    T& operator[](size_t i) { return ptr_[i]; }
    const T& operator[](size_t i) const { return ptr_[i]; }
    T& operator*() { return *ptr_; }
    const T& operator*() const { return *ptr_; }
    operator T*() { return ptr_; }
    operator const T*() const { return ptr_; }
    T* get() { return ptr_; }
    const T* get() const { return ptr_; }
    size_t size() const { return size_; }

    // Pointer arithmetic
    mmap operator+(size_t offset) const { return mmap(ptr_ + offset, size_ - offset); }
    T* begin() { return ptr_; }
    T* end() { return ptr_ + size_; }
    const T* begin() const { return ptr_; }
    const T* end() const { return ptr_ + size_; }
};

// Async memory-mapped interface (for burst access)
template<typename T>
class async_mmap {
    T* ptr_;
    size_t size_;
public:
    async_mmap() : ptr_(nullptr), size_(0) {}
    async_mmap(T* p, size_t n) : ptr_(p), size_(n) {}

    // Read address channel
    struct read_addr_t {
        bool try_write(uint64_t addr) { return true; }
        bool full() { return false; }
        void write(uint64_t addr) {}
    };
    // Read data channel
    struct read_data_t {
        bool empty() { return true; }
        bool try_read(T& val) { return false; }
        T read() { return T{}; }
        void open() {}
    };
    // Write address channel
    struct write_addr_t {
        bool try_write(uint64_t addr) { return true; }
        bool full() { return false; }
        void write(uint64_t addr) {}
    };
    // Write data channel
    struct write_data_t {
        bool try_write(const T& val) { return true; }
        bool full() { return false; }
        void write(const T& val) {}
    };
    // Write response channel
    struct write_resp_t {
        bool empty() { return true; }
        bool try_read(uint8_t& resp) { return false; }
        uint8_t read() { return 0; }
        void open() {}
    };

    read_addr_t read_addr;
    read_data_t read_data;
    write_addr_t write_addr;
    write_data_t write_data;
    write_resp_t write_resp;

    size_t size() const { return size_; }
};

// Memory-mapped vector
template<typename T>
class mmaps {
    std::vector<mmap<T>> maps_;
public:
    mmaps() {}
    mmaps(size_t n) : maps_(n) {}
    mmap<T>& operator[](size_t i) { return maps_[i]; }
    const mmap<T>& operator[](size_t i) const { return maps_[i]; }
    size_t size() const { return maps_.size(); }
};


// ---- Stream interfaces ----

template<typename T, int N = 0>
class stream {
    std::queue<T> data_;
    std::string name_;
public:
    stream() {}
    stream(const char* name) : name_(name) {}
    stream(const std::string& name) : name_(name) {}

    // Blocking operations
    T read() {
        if (data_.empty()) return T{};
        T val = data_.front();
        data_.pop();
        return val;
    }
    void read(T& val) { val = read(); }
    void write(const T& val) { data_.push(val); }

    // Non-blocking operations
    bool try_read(T& val) {
        if (data_.empty()) return false;
        val = data_.front();
        data_.pop();
        return true;
    }
    bool try_write(const T& val) {
        data_.push(val);
        return true;
    }

    // Peek
    T peek() const { return data_.empty() ? T{} : data_.front(); }
    bool try_peek(T& val) const {
        if (data_.empty()) return false;
        val = data_.front();
        return true;
    }

    // Status
    bool empty() const { return data_.empty(); }
    bool full() const { return false; }
    size_t size() const { return data_.size(); }

    // EOS support
    bool try_eot(T& val) { return try_read(val); }
    bool eot(T& val) { return try_read(val); }
    void close() {}
    void open() {}

    // Operator overloads
    void operator<<(const T& val) { write(val); }
    void operator>>(T& val) { val = read(); }
};

// Array of streams
template<typename T, int N>
class streams {
    stream<T> arr_[N > 0 ? N : 1];
public:
    streams() {}
    streams(const char* name) {
        for (int i = 0; i < N; ++i) arr_[i] = stream<T>(name);
    }
    stream<T>& operator[](size_t i) { return arr_[i]; }
    const stream<T>& operator[](size_t i) const { return arr_[i]; }
    static constexpr int size() { return N; }
};

// istream/ostream wrappers (some TAPA designs use directional streams)
template<typename T, int N = 0>
using istream = stream<T, N>;

template<typename T, int N = 0>
using ostream = stream<T, N>;

template<typename T, int N>
using istreams = streams<T, N>;

template<typename T, int N>
using ostreams = streams<T, N>;


// ---- Task invocation ----

class task {
public:
    // invoke() with various arities — just calls the function
    template<typename Func, typename... Args>
    task& invoke(Func&& func, Args&&... args) {
        return *this;
    }

    // invoke<Join/Detach>
    template<int Mode, typename Func, typename... Args>
    task& invoke(Func&& func, Args&&... args) {
        return *this;
    }

    // Seq/n variants
    template<typename T>
    static T seq() { return T{}; }

    // Wait for all tasks
    void wait() {}
};

// Execution modes
constexpr int join = 0;
constexpr int detach = 1;

// TAPA utility functions
template<typename T>
T reg(T val) { return val; }

template<typename T>
T bit_cast(uint64_t val) {
    T result;
    return result;
}

inline int widthof(int) { return 32; }
template<int W>
constexpr int widthof(const ap_int<W>&) { return W; }
template<int W>
constexpr int widthof(const ap_uint<W>&) { return W; }

} // namespace tapa

// Some TAPA designs use these macros
#ifndef TAPA_WHILE_NEITHER_EOS
#define TAPA_WHILE_NEITHER_EOS(...) while(true)
#endif

#ifndef TAPA_WHILE_NOT_EOS
#define TAPA_WHILE_NOT_EOS(...) while(true)
#endif

#endif // __TAPA_H__
