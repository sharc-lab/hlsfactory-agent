// Experiment 7 (docs/notes/xls_pragma_experiment_results.md).
// Loop pipelining: each call reads 8 values in a loop and writes their sum.
// Function-level II: same source as pipeline_loop_ii1.cc, but codegen is asked
// for --worst_case_throughput=2 (see .codegen_flags), the lead for Vitis
// "pipeline II=2" on a function.
class Block {
  __xls_channel<int, __xls_channel_dir_In> in;
  __xls_channel<int, __xls_channel_dir_Out> out;

  #pragma hls_top
  void foo() {
    int acc = 0;
    #pragma hls_pipeline_init_interval 1
    for (int i = 0; i < 8; ++i) {
      acc += in.read();
    }
    out.write(acc);
  }
};
