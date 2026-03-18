open_project k_compress_store_proj
set_top /output/SERI/k_compress_store/k_compress_store.cpp:k_compress_store
set_part xcu250-figd2104-2L-e
add_files {/output/SERI/k_compress_store/*.cpp}
add_files -tb {/output/SERI/k_compress_store/testbench.cpp}
open_solution "solution1"
csynth_design
exit
