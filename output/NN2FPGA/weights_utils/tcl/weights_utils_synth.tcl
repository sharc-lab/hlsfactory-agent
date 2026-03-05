# HLS Synthesis Script for weights_utils
# Auto-generated for NN2FPGA

open_project -reset weights_utils_proj
set_top weights_utils

# Add source files
add_files src/weights_utils.h

# Add testbench
add_files -tb testbench/tb_weights_utils.cpp

# Open solution
open_solution -reset "solution1"

# Set FPGA part (example: xczu3eg-sbva484-1-e for Ultra96)
set_part {xczu3eg-sbva484-1-e}

# Create clock
create_clock -period 5 -name default

# Run synthesis
csynth_design

# Export design
export_design -format ip_catalog

exit
