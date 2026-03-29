open_project /output/fpgaconvnet-hls/glue/glue_proj
set_top glue
add_files glue.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
