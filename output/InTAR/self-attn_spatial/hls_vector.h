/*
 * Stub header for Xilinx hls_vector.h
 * For compilation testing with clang++ only - NOT for HLS synthesis
 */

#ifndef __HLS_VECTOR_H__
#define __HLS_VECTOR_H__

#include <array>

namespace hls {

template<typename T, int N>
using vector = std::array<T, N>;

} // namespace hls

#endif // __HLS_VECTOR_H__
