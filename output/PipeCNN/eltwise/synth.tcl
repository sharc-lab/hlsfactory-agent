# Vivado HLS / Vitis HLS synthesis script for eltwise

open_project eltwise
set_top eltwise

add_files eltwise.cpp
add_files -tb testbench.cpp

open_solution "solution1"
set_part {xcu250-figd2104-2L-e}
create_clock -period 10 -name default

csynth_design
exit
