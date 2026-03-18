open_project /output/pasta/hbm-bandwidth//output/pasta/hbm-bandwidth/src/bandwidth.cpp:void Copy(tapa::async_mmap<Elem>& mem, uint64_t n, uint64_t flags) {
/output/pasta/hbm-bandwidth/src/bandwidth-host.cpp:void Bandwidth(tapa::mmaps<Elem, kBankCount> chan, uint64_t n, uint64_t flags);_proj
set_top /output/pasta/hbm-bandwidth/src/bandwidth.cpp:void Copy(tapa::async_mmap<Elem>& mem, uint64_t n, uint64_t flags) {
/output/pasta/hbm-bandwidth/src/bandwidth-host.cpp:void Bandwidth(tapa::mmaps<Elem, kBankCount> chan, uint64_t n, uint64_t flags);
add_files -cflags "-I/workspace/stubs"
add_files "/output/pasta/hbm-bandwidth/src/bandwidth.cpp"
add_files "/output/pasta/hbm-bandwidth/src/bandwidth-host.cpp"
add_files -tb "/output/pasta/hbm-bandwidth/src/bandwidth-host.cpp"
open_solution "solution1" -flow_target vitis
csynth_design
exit
