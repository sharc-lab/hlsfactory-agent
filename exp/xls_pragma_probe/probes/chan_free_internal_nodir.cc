// Experiment 8 (docs/notes/xls_pragma_experiment_results.md).
// stream_two_blocks.cc with no channel directions anywhere.
#pragma hls_design block
void producer(__xls_channel<int>& in, __xls_channel<int>& xfer) {
  xfer.write(in.read() + 1);
}

#pragma hls_design block
void consumer(__xls_channel<int>& out, __xls_channel<int>& xfer) {
  out.write(xfer.read() * 10);
}

#pragma hls_top
void top(__xls_channel<int>& in, __xls_channel<int>& out) {
  static __xls_channel<int> xfer;
  producer(in, xfer);
  consumer(out, xfer);
}
