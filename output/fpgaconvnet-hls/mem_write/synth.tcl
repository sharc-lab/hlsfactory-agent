open_project /output/fpgaconvnet-hls/mem_write/mem_write_proj
set_top mem_write
add_files mem_write.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
