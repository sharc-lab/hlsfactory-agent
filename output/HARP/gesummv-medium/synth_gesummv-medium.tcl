# HLS Synthesis Script for gesummv-medium
# Generated automatically by HLSFactory

open_project -reset gesummv-medium
set_top kernel_gesummv
add_files gesummv-medium_kernel.c
add_files -tb tb_gesummv-medium.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "gesummv-medium HLS design"

exit
