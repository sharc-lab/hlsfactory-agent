#ifndef HLSLIB_XILINX_FLATTEN_H
#define HLSLIB_XILINX_FLATTEN_H

#include <array>
#include <cstddef>

namespace hlslib {

template <long long Start0, long long End0, long long Step0, long long... Rest>
class ConstFlatten {
 public:
  static_assert((sizeof...(Rest) % 3) == 0,
                "ConstFlatten expects triples of start/end/step values.");

  static constexpr std::size_t kDims = (3 + sizeof...(Rest)) / 3;
  static constexpr std::array<long long, 3 + sizeof...(Rest)> kParams = {
      Start0, End0, Step0, Rest...};

  ConstFlatten() {
    for (std::size_t i = 0; i < kDims; ++i) {
      current_[i] = Start(i);
    }
  }

  std::size_t size() const {
    std::size_t total = 1;
    for (std::size_t i = 0; i < kDims; ++i) {
      total *= Extent(i);
    }
    return total;
  }

  long long operator[](std::size_t index) const { return current_[index]; }

  ConstFlatten& operator++() {
    for (std::size_t offset = 0; offset < kDims; ++offset) {
      const std::size_t i = kDims - 1 - offset;
      current_[i] += Step(i);
      if (current_[i] < End(i)) {
        break;
      }
      current_[i] = Start(i);
    }
    return *this;
  }

 private:
  static constexpr long long Start(std::size_t dim) { return kParams[dim * 3]; }
  static constexpr long long End(std::size_t dim) { return kParams[dim * 3 + 1]; }
  static constexpr long long Step(std::size_t dim) { return kParams[dim * 3 + 2]; }

  static constexpr std::size_t Extent(std::size_t dim) {
    const long long start = Start(dim);
    const long long end = End(dim);
    const long long step = Step(dim);
    return (step <= 0 || end <= start)
               ? 0
               : static_cast<std::size_t>((end - start + step - 1) / step);
  }

  std::array<long long, kDims> current_{};
};

template <long long Start0, long long End0, long long Step0, long long... Rest>
constexpr std::array<long long, 3 + sizeof...(Rest)>
    ConstFlatten<Start0, End0, Step0, Rest...>::kParams;

}  // namespace hlslib

#endif
