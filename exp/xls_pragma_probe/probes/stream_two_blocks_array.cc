// Experiment 4 (docs/notes/xls_pragma_experiment_results.md).
// Array variant: a whole int[4] travels through xfer as one item (wrapped in a
// struct). Vitis analogue: an array passed between DATAFLOW functions, which
// Vitis turns into a FIFO or ping-pong buffer (#pragma HLS stream / type=pipo).
struct Vec4 {
  int v[4];
};

#pragma hls_design block
void producer(__xls_channel<int, __xls_channel_dir_In>& in,
              __xls_channel<Vec4, __xls_channel_dir_InOut>& xfer) {
  const int x = in.read();
  Vec4 a;
  #pragma hls_unroll yes
  for (int i = 0; i < 4; ++i) {
    a.v[i] = x + i;
  }
  xfer.write(a);
}

#pragma hls_design block
void consumer(__xls_channel<int, __xls_channel_dir_Out>& out,
              __xls_channel<Vec4, __xls_channel_dir_InOut>& xfer) {
  const Vec4 a = xfer.read();
  int sum = 0;
  #pragma hls_unroll yes
  for (int i = 0; i < 4; ++i) {
    sum += a.v[i];
  }
  out.write(sum);
}

#pragma hls_top
void top(__xls_channel<int, __xls_channel_dir_In>& in,
         __xls_channel<int, __xls_channel_dir_Out>& out) {
  [[hls_fifo_depth(2)]] static __xls_channel<Vec4, __xls_channel_dir_InOut> xfer;
  producer(in, xfer);
  consumer(out, xfer);
}
