open_project /output/pasta/cnn//output/pasta/cnn/vitis_hls/src/kernel.c:int main(int argc, char **argv) {
/output/pasta/cnn/vitis_hls/src/kernel_host.hpp:  void deallocate(T* p, std::size_t num)
/output/pasta/cnn/vitis_hls/src/kernel_host.cpp:int main(int argc, char **argv) {
/output/pasta/cnn/vitis_hls/src/kernel_kernel.h:void A_IO_L2_in_intra_trans(int idx, A_t8 local_A[16][32], hls::stream<A_t8> &fifo_A_local_out, bool intra_trans_en);
/output/pasta/cnn/vitis_hls/src/kernel_kernel.cpp:void A_IO_L3_in(A_t16 *A, hls::stream<A_t8> &fifo_A_local_out){
/output/pasta/cnn/tapa/src/tapa_kernel.cpp:void A_IO_L3_in( tapa::mmap<A_t16> A, tapa::ostream<A_t8>& fifo_A_local_out ) {_proj
set_top /output/pasta/cnn/vitis_hls/src/kernel.c:int main(int argc, char **argv) {
/output/pasta/cnn/vitis_hls/src/kernel_host.hpp:  void deallocate(T* p, std::size_t num)
/output/pasta/cnn/vitis_hls/src/kernel_host.cpp:int main(int argc, char **argv) {
/output/pasta/cnn/vitis_hls/src/kernel_kernel.h:void A_IO_L2_in_intra_trans(int idx, A_t8 local_A[16][32], hls::stream<A_t8> &fifo_A_local_out, bool intra_trans_en);
/output/pasta/cnn/vitis_hls/src/kernel_kernel.cpp:void A_IO_L3_in(A_t16 *A, hls::stream<A_t8> &fifo_A_local_out){
/output/pasta/cnn/tapa/src/tapa_kernel.cpp:void A_IO_L3_in( tapa::mmap<A_t16> A, tapa::ostream<A_t8>& fifo_A_local_out ) {
add_files -cflags "-I/workspace/stubs"
add_files "/output/pasta/cnn/vitis_hls/src/kernel.c"
add_files "/output/pasta/cnn/vitis_hls/src/kernel_host.cpp"
add_files "/output/pasta/cnn/vitis_hls/src/kernel_kernel.cpp"
add_files "/output/pasta/cnn/tapa/src/tapa_kernel.cpp"
add_files -tb "/output/pasta/cnn/vitis_hls/src/kernel_host.cpp"
open_solution "solution1" -flow_target vitis
csynth_design
exit
