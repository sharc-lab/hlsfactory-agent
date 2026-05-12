open_project /output/fpgaconvnet-hls/relu/relu_proj
set_top relu
add_files relu.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
