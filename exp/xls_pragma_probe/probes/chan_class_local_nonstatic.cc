// Experiment 8 (docs/notes/xls_pragma_experiment_results.md).
// Internal channel as a non-static local. Expected: xlscc error.
class Block {
  __xls_channel<int, __xls_channel_dir_In> in;
  __xls_channel<int, __xls_channel_dir_Out> out;

  #pragma hls_top
  void foo() {
    __xls_channel<int> internal;  // not static
    internal.write(in.read());
    out.write(internal.read());
  }
};
