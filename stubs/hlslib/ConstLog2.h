#ifndef HLSLIB_CONSTLOG2_H
#define HLSLIB_CONSTLOG2_H

namespace hlslib {

constexpr int ConstLog2(unsigned long value, int acc = 0) {
  return value <= 1 ? acc : ConstLog2(value >> 1, acc + 1);
}

}  // namespace hlslib

#endif
