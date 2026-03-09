# Vivado HLS / Vitis HLS synthesis script for memWrite

open_project memWrite
set_top memWrite

add_files memWrite.cpp
add_files -tb testbench.cpp

open_solution "solution1"
set_part {xcu250-figd2104-2L-e}
create_clock -period 10 -name default

csynth_design
exit
