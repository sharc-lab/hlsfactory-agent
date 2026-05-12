open_project /output/pasta/spmm//output/pasta/spmm/sextans-u280-split-bram-uram/tapa/src/sextans-host.cpp:int main(int argc, char **argv) {
/output/pasta/spmm/sextans-u280-split-bram-uram/tapa/src/sparse_helper.h:int cmp_by_row_column(const void *aa,
/output/pasta/spmm/sextans-u280-split-bram-uram/tapa/src/modules.h:void async_read(tapa::async_mmap<T> & A,
/output/pasta/spmm/sextans-u280-split-bram-uram/tapa/src/sextans.h:void Sextans(tapa::mmap<int> edge_list_ptr,
/output/pasta/spmm/sextans-u280-split-bram-uram/tapa/src/sextans.cpp:void async_read(tapa::async_mmap<T> & A,
/output/pasta/spmm/sextans-u280-split-bram-uram/tapa/src/mmio.h:int mm_read_banner(FILE *f, MM_typecode *matcode);_proj
set_top /output/pasta/spmm/sextans-u280-split-bram-uram/tapa/src/sextans-host.cpp:int main(int argc, char **argv) {
/output/pasta/spmm/sextans-u280-split-bram-uram/tapa/src/sparse_helper.h:int cmp_by_row_column(const void *aa,
/output/pasta/spmm/sextans-u280-split-bram-uram/tapa/src/modules.h:void async_read(tapa::async_mmap<T> & A,
/output/pasta/spmm/sextans-u280-split-bram-uram/tapa/src/sextans.h:void Sextans(tapa::mmap<int> edge_list_ptr,
/output/pasta/spmm/sextans-u280-split-bram-uram/tapa/src/sextans.cpp:void async_read(tapa::async_mmap<T> & A,
/output/pasta/spmm/sextans-u280-split-bram-uram/tapa/src/mmio.h:int mm_read_banner(FILE *f, MM_typecode *matcode);
add_files -cflags "-I/workspace/stubs"
add_files "/output/pasta/spmm/sextans-u280-split-bram-uram/tapa/src/sextans-host.cpp"
add_files "/output/pasta/spmm/sextans-u280-split-bram-uram/tapa/src/sextans.cpp"
add_files -tb "/output/pasta/spmm/sextans-u280-split-bram-uram/tapa/src/sextans-host.cpp"
open_solution "solution1" -flow_target vitis
csynth_design
exit
