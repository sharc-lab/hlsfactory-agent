# HLS Synthesis Script for gemver
# Generated automatically by HLSFactory

open_project -reset gemver
set_top kernel_gemver
add_files gemver_kernel.c
add_files -tb tb_gemver.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "gemver HLS design"

exit
