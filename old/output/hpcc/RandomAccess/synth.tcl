# Vivado HLS/Vitis HLS Synthesis Script for RandomAccess

# Open project
open_project hpcc_random_access
set_top RandomAccessUpdate
add_files core_single_cpu.c
add_files RandomAccess.h
add_files -tb testbench.cpp

# Open solution
open_solution "solution1"
set_part {xcu250-figd2104-2L-e}
create_clock -period 4 -name default

# Run synthesis
csynth_design

# Exit
exit
