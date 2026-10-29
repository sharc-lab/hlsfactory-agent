// Experiment 5 (docs/notes/xls_pragma_experiment_results.md).
// Plain C array: does XLS keep it in registers with all elements readable in
// the same cycle (like Vitis array_partition complete), or make it a RAM?
// Each call writes one element, then reads all 8.
class Block {
  __xls_channel<unsigned, __xls_channel_dir_In> idx;
  __xls_channel<int, __xls_channel_dir_In> val;
  __xls_channel<int, __xls_channel_dir_Out> out;

  #pragma hls_top
  void foo() {
    static int arr[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    arr[idx.read() & 7] = val.read();

    int sum = 0;
    #pragma hls_unroll yes
    for (int i = 0; i < 8; ++i) {
      sum += arr[i];
    }
    out.write(sum);
  }
};
