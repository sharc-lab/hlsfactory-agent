# HLS Synthesis Script for gemver-medium
# Generated automatically by HLSFactory

open_project -reset gemver-medium
set_top kernel_gemver
add_files gemver-medium_kernel.c
add_files -tb tb_gemver-medium.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "gemver-medium HLS design"

exit
