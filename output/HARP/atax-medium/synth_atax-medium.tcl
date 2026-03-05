# HLS Synthesis Script for atax-medium
# Generated automatically by HLSFactory

open_project -reset atax-medium
set_top kernel_atax
add_files atax-medium_kernel.c
add_files -tb tb_atax-medium.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "atax-medium HLS design"

exit
