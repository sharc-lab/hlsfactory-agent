#ifndef HLSLIB_OP_H
#define HLSLIB_OP_H

namespace hlslib {
namespace op {

template <typename T>
struct Add {
  static T Apply(const T &a, const T &b) { return a + b; }
  static T apply(const T &a, const T &b) { return Apply(a, b); }
  static T identity() { return T{}; }
};

template <typename T>
struct Multiply {
  static T Apply(const T &a, const T &b) { return a * b; }
  static T apply(const T &a, const T &b) { return Apply(a, b); }
  static T identity() { return static_cast<T>(1); }
};

}  // namespace op
}  // namespace hlslib

#endif
