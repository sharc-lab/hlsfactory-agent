// Experiment 8 (docs/notes/xls_pragma_experiment_results.md).
// stream_two_blocks.cc, but the sub-blocks take xfer as Out (producer) and In
// (consumer) instead of InOut. Expected: C++ compile error.
#pragma hls_design block
void producer(__xls_channel<int, __xls_channel_dir_In>& in,
              __xls_channel<int, __xls_channel_dir_Out>& xfer) {
  xfer.write(in.read() + 1);
}

#pragma hls_design block
void consumer(__xls_channel<int, __xls_channel_dir_Out>& out,
              __xls_channel<int, __xls_channel_dir_In>& xfer) {
  out.write(xfer.read() * 10);
}

#pragma hls_top
void top(__xls_channel<int, __xls_channel_dir_In>& in,
         __xls_channel<int, __xls_channel_dir_Out>& out) {
  static __xls_channel<int, __xls_channel_dir_InOut> xfer;
  producer(in, xfer);
  consumer(out, xfer);
}
