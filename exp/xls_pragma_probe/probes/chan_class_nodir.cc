// Experiment 8 (docs/notes/xls_pragma_experiment_results.md).
// Port (class member) with no direction. Expected: xlscc error.
class Block {
  __xls_channel<int> in;  // no direction
  __xls_channel<int, __xls_channel_dir_Out> out;

  #pragma hls_top
  void foo() { out.write(in.read() * 2); }
};
