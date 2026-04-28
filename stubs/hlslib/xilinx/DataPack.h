#ifndef HLSLIB_XILINX_DATAPACK_H
#define HLSLIB_XILINX_DATAPACK_H

#include <cstddef>

namespace hlslib {

template <typename T, int N>
struct DataPack {
  static constexpr int kWidth = N;
  T data[N]{};

  DataPack() = default;
  explicit DataPack(T value) {
    for (int i = 0; i < N; ++i) {
      data[i] = value;
    }
  }

  T &operator[](std::size_t index) { return data[index]; }
  const T &operator[](std::size_t index) const { return data[index]; }

  void Pack(const T *src) {
    for (int i = 0; i < N; ++i) {
      data[i] = src[i];
    }
  }

  void Unpack(T *dst) const {
    for (int i = 0; i < N; ++i) {
      dst[i] = data[i];
    }
  }
};

}  // namespace hlslib

#endif
