// Experiment 8 (docs/notes/xls_pragma_experiment_results.md).
// A plain helper function and a helper method (no pragma). Do they become
// their own Verilog modules, or are they inlined into Block_proc?
int scale(int x) { return x * 3 + 1; }

class Block {
  __xls_channel<int, __xls_channel_dir_In> in;
  __xls_channel<int, __xls_channel_dir_Out> out;

  int offset(int x) { return x - 5; }

  #pragma hls_top
  void foo() { out.write(offset(scale(in.read()))); }
};
