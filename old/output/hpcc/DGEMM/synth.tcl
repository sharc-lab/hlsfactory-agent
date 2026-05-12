# Vivado HLS/Vitis HLS Synthesis Script for DGEMM

# Open project
open_project hpcc_dgemm
set_top dgemm_kernel
add_files tstdgemm.c
add_files -tb testbench.cpp

# Open solution
open_solution "solution1"
set_part {xcu250-figd2104-2L-e}
create_clock -period 4 -name default

# Run synthesis
csynth_design

# Exit
exit
