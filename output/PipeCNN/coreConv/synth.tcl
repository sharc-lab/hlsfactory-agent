# Vivado HLS / Vitis HLS synthesis script for coreConv

open_project coreConv
set_top coreConv

add_files coreConv.cpp
add_files -tb testbench.cpp

open_solution "solution1"
set_part {xcu250-figd2104-2L-e}
create_clock -period 10 -name default

csynth_design
exit
