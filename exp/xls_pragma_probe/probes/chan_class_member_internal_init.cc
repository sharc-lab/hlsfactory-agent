// Experiment 8 (docs/notes/xls_pragma_experiment_results.md).
// Same as chan_class_member_internal.cc, but with an in-class initializer.
// Expected: xlscc crashes (failed check at translator.cc:774).
class Block {
  __xls_channel<int, __xls_channel_dir_In> in;
  __xls_channel<int, __xls_channel_dir_Out> out;
  __xls_channel<int, __xls_channel_dir_InOut> internal =
      __xls_channel<int, __xls_channel_dir_InOut>();

  #pragma hls_top
  void foo() {
    internal.write(in.read() * 2);
    out.write(internal.read());
  }
};
