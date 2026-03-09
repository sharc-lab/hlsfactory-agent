# Vivado HLS / Vitis HLS synthesis script for memRead

# Open project
open_project memRead
set_top memRead

# Add source files
add_files memRead.cpp

# Add testbench files
add_files -tb testbench.cpp

# Open solution
open_solution "solution1"

# Set target FPGA device
set_part {xcu250-figd2104-2L-e}

# Create clock
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Exit
exit
