// Experiment 4 (docs/notes/xls_pragma_experiment_results.md).
// Vitis-style producer -> stream -> consumer, as two XLS blocks joined by an
// internal channel. Vitis analogue: two functions in a DATAFLOW region with an
// hls::stream between them. Here xfer has no depth pragma (default).
#pragma hls_design block
void producer(__xls_channel<int, __xls_channel_dir_In>& in,
              __xls_channel<int, __xls_channel_dir_InOut>& xfer) {
  xfer.write(in.read() + 1);
}

#pragma hls_design block
void consumer(__xls_channel<int, __xls_channel_dir_Out>& out,
              __xls_channel<int, __xls_channel_dir_InOut>& xfer) {
  out.write(xfer.read() * 10);
}

#pragma hls_top
void top(__xls_channel<int, __xls_channel_dir_In>& in,
         __xls_channel<int, __xls_channel_dir_Out>& out) {
  static __xls_channel<int, __xls_channel_dir_InOut> xfer;
  producer(in, xfer);
  consumer(out, xfer);
}
