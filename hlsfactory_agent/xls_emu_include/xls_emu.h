// Host-side emulation of the xlscc channel type so translated XLS designs compile and run with clang.
// Under xlscc (__SYNTHESIS__ is defined) this header is empty: xlscc provides __xls_channel itself.
#ifndef XLS_EMU_H
#define XLS_EMU_H

#ifndef __SYNTHESIS__
#include <deque>
#include <stdexcept>

template <typename T>
class __xls_channel {
  std::deque<T> q_;

 public:
  T read() {
    if (q_.empty()) throw std::runtime_error("read on empty __xls_channel");
    T v = q_.front();
    q_.pop_front();
    return v;
  }
  void write(const T &v) { q_.push_back(v); }
  bool nb_read(T &v) {
    if (q_.empty()) return false;
    v = q_.front();
    q_.pop_front();
    return true;
  }
  bool empty() const { return q_.empty(); }
};
#endif  // __SYNTHESIS__

#endif  // XLS_EMU_H
