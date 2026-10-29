// Experiment 7 (docs/notes/xls_pragma_experiment_results.md).
// Loop pipelining: each call reads 8 values in a loop and writes their sum.
// With II=1 the loop should take a new input every cycle. Here: no pragma (notes: block mode pipelines it at the top-level II, 1).
class Block {
  __xls_channel<int, __xls_channel_dir_In> in;
  __xls_channel<int, __xls_channel_dir_Out> out;

  #pragma hls_top
  void foo() {
    int acc = 0;
    for (int i = 0; i < 8; ++i) {
      acc += in.read();
    }
    out.write(acc);
  }
};
