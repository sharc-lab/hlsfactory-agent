open_project /output/fpgaconvnet-hls/elementwise_mul/elementwise_mul_proj
set_top elementwise_mul
add_files elementwise_mul.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
