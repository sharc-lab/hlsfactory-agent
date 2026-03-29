open_project /output/fpgaconvnet-hls/sliding_window/sliding_window_proj
set_top sliding_window
add_files sliding_window.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
