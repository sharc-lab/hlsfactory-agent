/*
 * Stub header for Xilinx hls_stream.h
 * For compilation testing with clang++ only - NOT for HLS synthesis
 *
 * This provides minimal definitions to allow code using hls::stream
 * to compile with standard compilers.
 */

#ifndef __HLS_STREAM_H__
#define __HLS_STREAM_H__

#include <queue>
#include <string>

namespace hls {

// DEPTH template parameter is used by some designs for FIFO sizing
template<typename T, int DEPTH = 0>
class stream {
private:
    std::queue<T> data;
    std::string name;

public:
    stream() : name("") {}
    stream(const char* n) : name(n) {}
    stream(const std::string& n) : name(n) {}

    // Write to stream
    void write(const T& val) {
        data.push(val);
    }

    // Blocking read from stream
    T read() {
        T val = data.front();
        data.pop();
        return val;
    }

    // Read with output parameter
    void read(T& val) {
        val = data.front();
        data.pop();
    }

    // Non-blocking read
    bool read_nb(T& val) {
        if (data.empty()) {
            return false;
        }
        val = data.front();
        data.pop();
        return true;
    }

    // Non-blocking write (always succeeds in simulation)
    bool write_nb(const T& val) {
        data.push(val);
        return true;
    }

    // Check if empty
    bool empty() const {
        return data.empty();
    }

    // Check if full (never full in simulation)
    bool full() const {
        return false;
    }

    // Get size
    size_t size() const {
        return data.size();
    }

    // Operator overloads for convenience
    void operator<<(const T& val) {
        write(val);
    }

    void operator>>(T& val) {
        val = read();
    }
};

} // namespace hls

#endif // __HLS_STREAM_H__
