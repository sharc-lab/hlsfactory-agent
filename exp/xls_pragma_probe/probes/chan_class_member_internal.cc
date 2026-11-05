// Experiment 8 (docs/notes/xls_pragma_experiment_results.md).
// Internal channel as a class member, set up in the constructor, instead of a
// static local. Expected: xlscc error.
class Block {
 public:
  __xls_channel<int, __xls_channel_dir_In> in;
  __xls_channel<int, __xls_channel_dir_Out> out;
  __xls_channel<int, __xls_channel_dir_InOut> internal;
  Block() : internal() {}

  #pragma hls_top
  void foo() {
    internal.write(in.read() * 2);
    out.write(internal.read());
  }
};
