open_project /output/fpgaconvnet-hls/mem_read/mem_read_proj
set_top mem_read
add_files mem_read.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
