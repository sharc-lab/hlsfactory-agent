open_project /output/pasta/page_rank//output/pasta/page_rank/tapa/src/page-rank-host.cpp:void PageRank(Pid num_partitions, tapa::mmap<uint64_t> metadata,
/output/pasta/page_rank/tapa/src/nxgraph.hpp:  void LoadAttr(const char* ptr, const char** next_ptr) {
/output/pasta/page_rank/tapa/src/page-rank.cpp:void Control(Pid num_partitions, tapa::mmap<uint64_t> metadata,_proj
set_top /output/pasta/page_rank/tapa/src/page-rank-host.cpp:void PageRank(Pid num_partitions, tapa::mmap<uint64_t> metadata,
/output/pasta/page_rank/tapa/src/nxgraph.hpp:  void LoadAttr(const char* ptr, const char** next_ptr) {
/output/pasta/page_rank/tapa/src/page-rank.cpp:void Control(Pid num_partitions, tapa::mmap<uint64_t> metadata,
add_files -cflags "-I/workspace/stubs"
add_files "/output/pasta/page_rank/tapa/src/page-rank-host.cpp"
add_files "/output/pasta/page_rank/tapa/src/page-rank.cpp"
add_files -tb "/output/pasta/page_rank/tapa/src/page-rank-host.cpp"
open_solution "solution1" -flow_target vitis
csynth_design
exit
