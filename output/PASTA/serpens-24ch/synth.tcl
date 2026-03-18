open_project /output/pasta/serpens-24ch//output/pasta/serpens-24ch/tapa/src/serpens.h:void Serpens(tapa::mmap<int> edge_list_ptr,
/output/pasta/serpens-24ch/tapa/src/serpens.cpp:void read_edge_list_ptr(const int num_ite,
/output/pasta/serpens-24ch/tapa/src/serpens-host.cpp:int main(int argc, char **argv) {
/output/pasta/serpens-24ch/tapa/src/sparse_helper.h:int cmp_by_row_column(const void *aa,
/output/pasta/serpens-24ch/tapa/src/mmio.h:int mm_read_banner(FILE *f, MM_typecode *matcode);_proj
set_top /output/pasta/serpens-24ch/tapa/src/serpens.h:void Serpens(tapa::mmap<int> edge_list_ptr,
/output/pasta/serpens-24ch/tapa/src/serpens.cpp:void read_edge_list_ptr(const int num_ite,
/output/pasta/serpens-24ch/tapa/src/serpens-host.cpp:int main(int argc, char **argv) {
/output/pasta/serpens-24ch/tapa/src/sparse_helper.h:int cmp_by_row_column(const void *aa,
/output/pasta/serpens-24ch/tapa/src/mmio.h:int mm_read_banner(FILE *f, MM_typecode *matcode);
add_files -cflags "-I/workspace/stubs"
add_files "/output/pasta/serpens-24ch/tapa/src/serpens.cpp"
add_files "/output/pasta/serpens-24ch/tapa/src/serpens-host.cpp"
add_files -tb "/output/pasta/serpens-24ch/tapa/src/serpens-host.cpp"
open_solution "solution1" -flow_target vitis
csynth_design
exit
