open_project /output/fpgaconvnet-hls/elementwise_add/elementwise_add_proj
set_top elementwise_add
add_files elementwise_add.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
