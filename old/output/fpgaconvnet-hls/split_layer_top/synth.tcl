open_project /output/fpgaconvnet-hls/split_layer_top/split_layer_top_proj
set_top split_layer_top
add_files split_layer_top.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
