// Experiment 2 (docs/notes/xls_pragma_experiment_results.md).
// What depth does an internal channel get with no [[hls_fifo_depth]]?
class Block {
  __xls_channel<int, __xls_channel_dir_In> in;
  __xls_channel<int, __xls_channel_dir_Out> out;

  #pragma hls_top
  void foo() {
    static __xls_channel<int> internal;  // no depth pragma

    const int x = in.read();
    internal.write(x * 2);

    const int xr = internal.read();
    out.write(xr);
  }
};
