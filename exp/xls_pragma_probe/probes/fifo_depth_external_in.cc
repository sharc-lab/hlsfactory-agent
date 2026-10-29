// Experiment 3 (docs/notes/xls_pragma_experiment_results.md).
// [[hls_fifo_depth(4)]] on the external *in* channel (a class member that
// becomes module ports): error, warning, or silently ignored?
class Block {
  [[hls_fifo_depth(4)]] __xls_channel<int, __xls_channel_dir_In> in;
  __xls_channel<int, __xls_channel_dir_Out> out;

  #pragma hls_top
  void foo() {
    const int x = in.read();
    out.write(x * 2);
  }
};
