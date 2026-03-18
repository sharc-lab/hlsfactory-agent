/*
 * Stub header for Xilinx hls_vector.h
 * For compilation testing with clang++ only - NOT for HLS synthesis
 */

#ifndef __HLS_VECTOR_H__
#define __HLS_VECTOR_H__

namespace hls {

template<typename T, int N>
struct vector {
    T data[N];
    T& operator[](int i) { return data[i]; }
    const T& operator[](int i) const { return data[i]; }
};

} // namespace hls

#endif
