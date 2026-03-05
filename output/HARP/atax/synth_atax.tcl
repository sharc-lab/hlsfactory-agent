# HLS Synthesis Script for atax
# Generated automatically by HLSFactory

open_project -reset atax
set_top kernel_atax
add_files atax_kernel.c
add_files -tb tb_atax.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "atax HLS design"

exit
