# Vivado HLS / Vitis HLS synthesis script for maxPool

open_project maxPool
set_top maxPool

add_files maxPool.cpp
add_files -tb testbench.cpp

open_solution "solution1"
set_part {xcu250-figd2104-2L-e}
create_clock -period 10 -name default

csynth_design
exit
