open_project /output/pasta/lu_decompose//output/pasta/lu_decompose/hls/src/kernel.c:void init_array(data_t A[N][N])
/output/pasta/lu_decompose/hls/src/kernel_host.cpp:void init_array(data_t A[N][N])
/output/pasta/lu_decompose/hls/src/kernel_kernel.h:void kernel0(A_t1 *A, L_t1 *L, U_t16 *U);
/output/pasta/lu_decompose/hls/src/kernel_kernel.cpp:void A_IO_L3_in(A_t1 *A, hls::stream<float> &fifo_A_local_out) {
/output/pasta/lu_decompose/tapa/src/kernel_kernel.cpp:void PE_non_zero_non_diag(_proj
set_top /output/pasta/lu_decompose/hls/src/kernel.c:void init_array(data_t A[N][N])
/output/pasta/lu_decompose/hls/src/kernel_host.cpp:void init_array(data_t A[N][N])
/output/pasta/lu_decompose/hls/src/kernel_kernel.h:void kernel0(A_t1 *A, L_t1 *L, U_t16 *U);
/output/pasta/lu_decompose/hls/src/kernel_kernel.cpp:void A_IO_L3_in(A_t1 *A, hls::stream<float> &fifo_A_local_out) {
/output/pasta/lu_decompose/tapa/src/kernel_kernel.cpp:void PE_non_zero_non_diag(
add_files -cflags "-I/workspace/stubs"
add_files "/output/pasta/lu_decompose/hls/src/kernel.c"
add_files "/output/pasta/lu_decompose/hls/src/kernel_host.cpp"
add_files "/output/pasta/lu_decompose/hls/src/kernel_kernel.cpp"
add_files "/output/pasta/lu_decompose/tapa/src/kernel_kernel.cpp"
add_files -tb "/output/pasta/lu_decompose/hls/src/kernel_host.cpp"
open_solution "solution1" -flow_target vitis
csynth_design
exit
