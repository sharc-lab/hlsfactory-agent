open_project /output/fpgaconvnet-hls/avg_pool/avg_pool_proj
set_top avg_pool
add_files avg_pool.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
