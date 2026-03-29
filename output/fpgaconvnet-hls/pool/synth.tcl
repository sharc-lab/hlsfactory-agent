open_project /output/fpgaconvnet-hls/pool/pool_proj
set_top pool
add_files pool.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
