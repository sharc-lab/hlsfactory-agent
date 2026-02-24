# HLS Synthesis Script for FORC
# TCL script for Vitis HLS synthesis

# Create project
open_project forc_hls

# Set top function
set_top orc_proc

# Add source files
add_files kernel/orc_proc.cpp -cflags "-I./kernel -I."
add_files kernel/orcDecomp.h
add_files kernel/zlibTapa.h
add_files kernel/orc_decoder.h
add_files kernel/orc_filter.h
add_files kernel/orc_proc.h
add_files kernel/fixed_codes.hpp
add_files kernel/ap_int.h
add_files kernel/hls_stream.h

# Add testbench files (if available)
add_files -tb host/orc_proc_host.cpp -cflags "-I./host"

# Open solution
open_solution "solution1"

# Set target device
set_part {xcu280-fsvh2892-2L-e}

# Create clock
create_clock -period 3.0 -name default

# Run C simulation (optional, skip if no testbench)
# csim_design

# Run C synthesis
csynth_design

# Export RTL
cosim_design
export_design -rtl verilog -format ip_catalog

# Close project
close_project

exit
