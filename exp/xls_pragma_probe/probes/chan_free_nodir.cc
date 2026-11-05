// Experiment 8 (docs/notes/xls_pragma_experiment_results.md).
// Ports as top-function parameters with no direction; the .textproto gives it.
#pragma hls_top
void top(__xls_channel<int>& in, __xls_channel<int>& out) {
  out.write(in.read() * 2);
}
