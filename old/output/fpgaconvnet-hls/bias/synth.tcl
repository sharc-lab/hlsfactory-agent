open_project /output/fpgaconvnet-hls/bias/bias_proj
set_top bias
add_files bias.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
