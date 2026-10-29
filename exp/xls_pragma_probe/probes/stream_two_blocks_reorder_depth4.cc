// Experiment 6 (docs/notes/xls_pragma_experiment_results.md).
// Classic dataflow deadlock test. The producer writes a burst of 4 to xfer,
// THEN one value to side; the consumer reads side FIRST, then the 4 from xfer.
// This only completes if xfer can hold the whole burst. Here xfer has depth 4 (exactly the burst).
#pragma hls_design block
void producer(__xls_channel<int, __xls_channel_dir_In>& in,
              __xls_channel<int, __xls_channel_dir_InOut>& xfer,
              __xls_channel<int, __xls_channel_dir_InOut>& side) {
  const int x = in.read();
  #pragma hls_unroll yes
  for (int i = 0; i < 4; ++i) {
    xfer.write(x + i);
  }
  side.write(x);
}

#pragma hls_design block
void consumer(__xls_channel<int, __xls_channel_dir_Out>& out,
              __xls_channel<int, __xls_channel_dir_InOut>& xfer,
              __xls_channel<int, __xls_channel_dir_InOut>& side) {
  int sum = side.read();
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
  static __xls_channel<int, __xls_channel_dir_InOut> side;
  producer(in, xfer, side);
  consumer(out, xfer, side);
}
