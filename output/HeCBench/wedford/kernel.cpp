#include "kernel.h"

// --- from main.cu ---
void welford_merge_element(C& count,
                           T& mean,
                           T& m2n,
                           const C& num_new,
                           const T& mean_new,
                           const T& m2n_new) {
  T factor = T(1.0) / max(1, (count + num_new));
  T delta0 = mean - mean_new;
  mean = (mean_new * num_new + mean * count) * factor;
  m2n += m2n_new + delta0 * delta0 * num_new * count * factor;
  count += num_new;
}

void warp_reduce_mean_m2n(T &mean, T &m2n, int &num)
{
  #pragma unroll
  for(int i = warpSize/2; i > 0; i >>= 1) {
    auto num_new = 0;
    auto mean_new = 0;
    auto m2n_new = 0;
    welford_merge_element(num, mean, m2n, num_new, mean_new, m2n_new);
  }
}

void welford_reduce_mean_m2n(
      T*  x,
      int*  count,
      T &mean,
      T &m2n,
      int &num,
      int block_size,
      int thread_id)
{
  int lane = thread_id % warpSize;
  int wid = thread_id / warpSize;

  if (block_size > warpSize) {
    warp_reduce_mean_m2n(mean, m2n, num);
    if (lane == 0) {
      x[wid*2] = mean;
      x[wid*2+1] = m2n;
      count[wid] = num;
    }

    if (wid == 0) {
      mean = (thread_id < block_size / warpSize)? x[lane*2] : T(0);
      m2n = (thread_id < block_size / warpSize)? x[lane*2+1] : T(0);
      num = (thread_id < block_size / warpSize)? count[lane] : int(0);
    }
  }

  if (wid==0) warp_reduce_mean_m2n(mean, m2n, num);
}
extern "C"

void welford_kernel(
      const scalar_t*  input,
      outscalar_t*  out_mean,
      outscalar_t*  out_var_biased,
      const int bs,
      const int fs,
      const int ss)
{
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out_mean offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=out_var_biased offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=bs
    #pragma HLS INTERFACE s_axilite port=fs
    #pragma HLS INTERFACE s_axilite port=ss
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_mem complete dim=1

    for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                int block_size = BLOCK_DIM_X * BLOCK_DIM_Y;
                int count = 0;
                accscalar_t x_mean = accscalar_t(0);
                accscalar_t m_2_n = accscalar_t(0);

                int thread_id = _tid_y*BLOCK_DIM_X + _tid_x;

                for (int batch_id = _tid_y; batch_id < bs; batch_id += BLOCK_DIM_Y) {
                int input_base = _bid_x*ss + batch_id*ss*fs;
                for (int offset = _tid_x; offset < ss ; offset += BLOCK_DIM_X) {
                count++;
                auto x_n = static_cast<accscalar_t>(input[offset+input_base]);
                // sequential welford
                auto d = x_n - x_mean;
                x_mean += d / count;
                m_2_n += d * (x_n - x_mean);
                }
                }

                static int s_mem[160];
                accscalar_t* s_mem_ac = (accscalar_t*) &s_mem[32];

                welford_reduce_mean_m2n<accscalar_t>(s_mem_ac, s_mem, x_mean, m_2_n, count, block_size, thread_id);

                if (thread_id == 0) {
                out_mean[_bid_x] = static_cast<outscalar_t>(x_mean);
                out_var_biased[_bid_x] = static_cast<outscalar_t>(m_2_n/count);
                }

            }
        }
    }
}
