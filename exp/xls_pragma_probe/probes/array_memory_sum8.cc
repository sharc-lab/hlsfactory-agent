// Experiment 5 (docs/notes/xls_pragma_experiment_results.md).
// Same program as array_plain_sum8.cc, but the array is an explicit RAM
// (__xls_memory). Contrast: how are the 8 reads scheduled, and what ports appear?
class Block {
  __xls_channel<unsigned, __xls_channel_dir_In> idx;
  __xls_channel<int, __xls_channel_dir_In> val;
  __xls_channel<int, __xls_channel_dir_Out> out;
  __xls_memory<int, 8> arr;

  #pragma hls_top
  void foo() {
    arr[idx.read() & 7] = val.read();

    int sum = 0;
    #pragma hls_unroll yes
    for (int i = 0; i < 8; ++i) {
      sum += arr[i];
    }
    out.write(sum);
  }
};
