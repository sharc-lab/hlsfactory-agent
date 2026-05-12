open_project /output/fpgaconvnet-hls/global_pool/global_pool_proj
set_top global_pool
add_files global_pool.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
