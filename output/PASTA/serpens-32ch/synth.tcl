open_project /output/pasta/serpens-32ch//output/pasta/serpens-32ch/hls/src/host.cpp:int main(int argc, char **argv) {
/output/pasta/serpens-32ch/hls/src/serpens.cpp:float uint32_to_float(ap_uint<32> u) {
/output/pasta/serpens-32ch/hls/src/sparse_helper.h:int cmp_by_row_column(const void *aa,
/output/pasta/serpens-32ch/hls/src/mmio.h:int mm_read_banner(FILE *f, MM_typecode *matcode);
/output/pasta/serpens-32ch/tapa/src/serpens_tapa.cpp:float uint32_to_float(ap_uint<32> u) {
/output/pasta/serpens-32ch/tapa/src/serpens_tapa_mmap.cpp:float uint32_to_float(ap_uint<32> u) {
/output/pasta/serpens-32ch/tapa/src/mmio.h:int mm_read_banner(FILE *f, MM_typecode *matcode);_proj
set_top /output/pasta/serpens-32ch/hls/src/host.cpp:int main(int argc, char **argv) {
/output/pasta/serpens-32ch/hls/src/serpens.cpp:float uint32_to_float(ap_uint<32> u) {
/output/pasta/serpens-32ch/hls/src/sparse_helper.h:int cmp_by_row_column(const void *aa,
/output/pasta/serpens-32ch/hls/src/mmio.h:int mm_read_banner(FILE *f, MM_typecode *matcode);
/output/pasta/serpens-32ch/tapa/src/serpens_tapa.cpp:float uint32_to_float(ap_uint<32> u) {
/output/pasta/serpens-32ch/tapa/src/serpens_tapa_mmap.cpp:float uint32_to_float(ap_uint<32> u) {
/output/pasta/serpens-32ch/tapa/src/mmio.h:int mm_read_banner(FILE *f, MM_typecode *matcode);
add_files -cflags "-I/workspace/stubs"
add_files "/output/pasta/serpens-32ch/hls/src/host.cpp"
add_files "/output/pasta/serpens-32ch/hls/src/serpens.cpp"
add_files "/output/pasta/serpens-32ch/tapa/src/serpens_tapa.cpp"
add_files "/output/pasta/serpens-32ch/tapa/src/serpens_tapa_mmap.cpp"
add_files -tb "/output/pasta/serpens-32ch/hls/src/host.cpp"
open_solution "solution1" -flow_target vitis
csynth_design
exit
