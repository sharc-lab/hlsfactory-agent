open_project /output/pasta/spmv-hisparse-mmap//output/pasta/spmv-hisparse-mmap/hls/src/host.cpp:void compute_ref(
/output/pasta/spmv-hisparse-mmap/hls/src/data_formatter.h:void util_round_csr_matrix_dim(CSRMatrix<DataT> &csr_matrix,
/output/pasta/spmv-hisparse-mmap/hls/src/libfpga/vecbuf_access_unit.h:void vecbuf_reader(
/output/pasta/spmv-hisparse-mmap/hls/src/libfpga/stream_utils.h:void axis_duplicate(
/output/pasta/spmv-hisparse-mmap/hls/src/libfpga/spmv_cluster.h:void CPSR_matrix_loader(
/output/pasta/spmv-hisparse-mmap/hls/src/libfpga/shuffle.h:void arbiter_1p(
/output/pasta/spmv-hisparse-mmap/hls/src/libfpga/pe.h:void ufixed_pe_process(
/output/pasta/spmv-hisparse-mmap/hls/src/spmv.cpp:void CPSR_matrix_loader(
/output/pasta/spmv-hisparse-mmap/tapa/src/spmv.cpp:    void CPSR_matrix_loader(_proj
set_top /output/pasta/spmv-hisparse-mmap/hls/src/host.cpp:void compute_ref(
/output/pasta/spmv-hisparse-mmap/hls/src/data_formatter.h:void util_round_csr_matrix_dim(CSRMatrix<DataT> &csr_matrix,
/output/pasta/spmv-hisparse-mmap/hls/src/libfpga/vecbuf_access_unit.h:void vecbuf_reader(
/output/pasta/spmv-hisparse-mmap/hls/src/libfpga/stream_utils.h:void axis_duplicate(
/output/pasta/spmv-hisparse-mmap/hls/src/libfpga/spmv_cluster.h:void CPSR_matrix_loader(
/output/pasta/spmv-hisparse-mmap/hls/src/libfpga/shuffle.h:void arbiter_1p(
/output/pasta/spmv-hisparse-mmap/hls/src/libfpga/pe.h:void ufixed_pe_process(
/output/pasta/spmv-hisparse-mmap/hls/src/spmv.cpp:void CPSR_matrix_loader(
/output/pasta/spmv-hisparse-mmap/tapa/src/spmv.cpp:    void CPSR_matrix_loader(
add_files -cflags "-I/workspace/stubs"
add_files "/output/pasta/spmv-hisparse-mmap/hls/src/host.cpp"
add_files "/output/pasta/spmv-hisparse-mmap/hls/src/spmv.cpp"
add_files "/output/pasta/spmv-hisparse-mmap/tapa/src/spmv.cpp"
add_files -tb "/output/pasta/spmv-hisparse-mmap/hls/src/host.cpp"
open_solution "solution1" -flow_target vitis
csynth_design
exit
