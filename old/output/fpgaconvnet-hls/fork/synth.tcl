open_project /output/fpgaconvnet-hls/fork/fork_proj
set_top fork
add_files fork.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
