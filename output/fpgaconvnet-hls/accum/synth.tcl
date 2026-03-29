open_project /output/fpgaconvnet-hls/accum/accum_proj
set_top accum
add_files accum.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
