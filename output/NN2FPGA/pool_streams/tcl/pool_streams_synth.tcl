# HLS Synthesis Script for pool_streams
# Auto-generated for NN2FPGA

open_project -reset pool_streams_proj
set_top pool_streams

# Add source files
add_files src/pool_streams.h

# Add testbench
add_files -tb testbench/tb_pool_streams.cpp

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
