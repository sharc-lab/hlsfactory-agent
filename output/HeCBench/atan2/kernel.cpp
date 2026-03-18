#include "kernel.h"

// --- from main.cu ---
constexpr float approx_atan2f_P(float x);

// degree =  3   => absolute accuracy is  7 bits
template <>
constexpr float approx_atan2f_P<3>(float x) {
  return x * (float(-0xf.8eed2p-4) + x * x * float(0x3.1238p-4));
}

constexpr float unsafe_atan2f_impl(float y, float x) {
  constexpr float pi4f = 3.1415926535897932384626434 / 4;
  constexpr float pi34f = 3.1415926535897932384626434 * 3 / 4;

  auto r = (std::abs(x) - std::abs(y)) / (std::abs(x) + std::abs(y));
  if (x < 0)
    r = -r;

  auto angle = (x >= 0) ? pi4f : pi34f;
  angle += approx_atan2f_P<DEGREE>(r);

  return ((y < 0)) ? -angle : angle;
}

constexpr float unsafe_atan2f(float y, float x) {
  return unsafe_atan2f_impl<DEGREE>(y, x);
}

constexpr float safe_atan2f(float y, float x) {
  return unsafe_atan2f_impl<DEGREE>(y, ((y == 0.f) & (x == 0.f)) ? 0.2f : x);
}

constexpr float approx_atan2i_P(float x);

// degree =  3   => absolute accuracy is  6*10^6
template <>
constexpr float approx_atan2i_P<3>(float x) {
  auto z = x * x;
  return x * (-664694912.f + z * 131209024.f);
}

constexpr int unsafe_atan2i_impl(float y, float x) {
  constexpr long long maxint = (long long)(std::numeric_limits<int>::max()) + 1LL;
  constexpr int pi4 = int(maxint / 4LL);
  constexpr int pi34 = int(3LL * maxint / 4LL);

  auto r = (std::abs(x) - std::abs(y)) / (std::abs(x) + std::abs(y));
  if (x < 0)
    r = -r;

  auto angle = (x >= 0) ? pi4 : pi34;
  angle += int(approx_atan2i_P<DEGREE>(r));

  return (y < 0) ? -angle : angle;
}

constexpr int unsafe_atan2i(float y, float x) {
  return unsafe_atan2i_impl<DEGREE>(y, x);
}

constexpr float approx_atan2s_P(float x);

// degree =  3   => absolute accuracy is  53
template <>
constexpr float approx_atan2s_P<3>(float x) {
  auto z = x * x;
  return x * ((-10142.439453125f) + z * 2002.0908203125f);
}

constexpr short unsafe_atan2s_impl(float y, float x) {
  constexpr int maxshort = (int)(std::numeric_limits<short>::max()) + 1;
  constexpr short pi4 = short(maxshort / 4);
  constexpr short pi34 = short(3 * maxshort / 4);

  auto r = (std::abs(x) - std::abs(y)) / (std::abs(x) + std::abs(y));
  if (x < 0)
    r = -r;

  auto angle = (x >= 0) ? pi4 : pi34;
  angle += short(approx_atan2s_P<DEGREE>(r));

  return (y < 0) ? -angle : angle;
}

constexpr short unsafe_atan2s(float y, float x) {
  return unsafe_atan2s_impl<DEGREE>(y, x);
}
extern "C"

void compute_f (const int n,
                const float *x,
                const float *y,
                      float *r)
{
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=r offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i >= n) return;
            const float vy = y[i];
            const float vx = x[i];
            r[i] = safe_atan2f< 3>(vy, vx) +
            safe_atan2f< 5>(vy, vx) +
            safe_atan2f< 7>(vy, vx) +
            safe_atan2f< 9>(vy, vx) +
            safe_atan2f<11>(vy, vx) +
            safe_atan2f<13>(vy, vx) +
            safe_atan2f<15>(vy, vx);

        }
    }
}
extern "C"

void compute_s (const int n,
                const float *x,
                const float *y,
                      short *r)
{
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=r offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i >= n) return;
            const float vy = y[i];
            const float vx = x[i];
            r[i] = unsafe_atan2s< 3>(vy, vx) +
            unsafe_atan2s< 5>(vy, vx) +
            unsafe_atan2s< 7>(vy, vx) +
            unsafe_atan2s< 9>(vy, vx);

        }
    }
}
extern "C"

void compute_i (const int n,
                const float *x,
                const float *y,
                      int *r)
{
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=r offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i >= n) return;
            const float vy = y[i];
            const float vx = x[i];
            r[i] = unsafe_atan2i< 3>(vy, vx) +
            unsafe_atan2i< 5>(vy, vx) +
            unsafe_atan2i< 7>(vy, vx) +
            unsafe_atan2i< 9>(vy, vx) +
            unsafe_atan2i<11>(vy, vx) +
            unsafe_atan2i<13>(vy, vx) +
            unsafe_atan2i<15>(vy, vx);

        }
    }
}
