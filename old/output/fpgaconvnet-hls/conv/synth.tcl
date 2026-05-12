open_project /output/fpgaconvnet-hls/conv/conv_proj
set_top conv
add_files conv.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
