# Vivado HLS/Vitis HLS Synthesis Script for STREAM

# Open project
open_project hpcc_stream
set_top main
add_files stream.c

add_files -tb testbench.cpp

# Open solution
open_solution "solution1"
set_part {xcu250-figd2104-2L-e}
create_clock -period 4 -name default

# Run synthesis
csynth_design

# Exit
exit
