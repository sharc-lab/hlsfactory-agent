#ifndef HLS_FFT_H
#define HLS_FFT_H

// HLS FFT library stub for compilation
// This is a minimal stub to allow clang compilation

#include <complex>

namespace hls {

template<typename T, int N>
class fft {
public:
    void run(std::complex<T> in[N], std::complex<T> out[N]) {}
};

} // namespace hls

#endif
