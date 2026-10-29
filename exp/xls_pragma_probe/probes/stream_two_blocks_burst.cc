// Experiment 4 (docs/notes/xls_pragma_experiment_results.md).
// Burst variant: the producer writes 4 values per input and the consumer reads
// all 4 and sums them, so several values can be in flight in xfer at once.
// This is the case where a Vitis stream depth matters.
#pragma hls_design block
void producer(__xls_channel<int, __xls_channel_dir_In>& in,
              __xls_channel<int, __xls_channel_dir_InOut>& xfer) {
  const int x = in.read();
  #pragma hls_unroll yes
  for (int i = 0; i < 4; ++i) {
    xfer.write(x + i);
  }
}

#pragma hls_design block
void consumer(__xls_channel<int, __xls_channel_dir_Out>& out,
              __xls_channel<int, __xls_channel_dir_InOut>& xfer) {
  int sum = 0;
  #pragma hls_unroll yes
  for (int i = 0; i < 4; ++i) {
    sum += xfer.read();
  }
  out.write(sum);
}

#pragma hls_top
void top(__xls_channel<int, __xls_channel_dir_In>& in,
         __xls_channel<int, __xls_channel_dir_Out>& out) {
  [[hls_fifo_depth(4)]] static __xls_channel<int, __xls_channel_dir_InOut> xfer;
  producer(in, xfer);
  consumer(out, xfer);
}
