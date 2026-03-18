open_project /output/pasta/knn//output/pasta/knn/src/knn.cpp:void load(int flag, int tile_idx, INTERFACE_WIDTH* local_SP, tapa::async_mmap<INTERFACE_WIDTH>& searchSpace)
/output/pasta/knn/src/knn-host.cpp:void Knn(_proj
set_top /output/pasta/knn/src/knn.cpp:void load(int flag, int tile_idx, INTERFACE_WIDTH* local_SP, tapa::async_mmap<INTERFACE_WIDTH>& searchSpace)
/output/pasta/knn/src/knn-host.cpp:void Knn(
add_files -cflags "-I/workspace/stubs"
add_files "/output/pasta/knn/src/knn.cpp"
add_files "/output/pasta/knn/src/knn-host.cpp"
add_files -tb "/output/pasta/knn/src/knn-host.cpp"
open_solution "solution1" -flow_target vitis
csynth_design
exit
