// Probe: does [[hls_fifo_depth(N)]] on an internal channel reach the IR?
class Block {
  __xls_channel<int, __xls_channel_dir_In> in;
  __xls_channel<int, __xls_channel_dir_Out> out;

  #pragma hls_top
  void foo() {
    [[hls_fifo_depth(4)]]
    static __xls_channel<int> internal;

    const int x = in.read();
    internal.write(x * 2);

    const int xr = internal.read();
    out.write(xr);
  }
};
