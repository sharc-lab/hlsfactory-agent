#ifndef HLSLIB_XILINX_STREAM_H
#define HLSLIB_XILINX_STREAM_H

namespace hlslib {

template <typename T, int Depth = 2>
class Stream {
 public:
  Stream(const char * = nullptr) {}

  bool empty() const { return true; }
  bool read_nb(T &value) {
    value = T{};
    return false;
  }

  T read() { return T{}; }
  T Pop() { return T{}; }

  void write(const T &) {}
  void Push(const T &) {}
  void set_name(const char *) {}
};

}  // namespace hlslib

#endif
