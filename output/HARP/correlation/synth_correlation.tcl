# HLS Synthesis Script for correlation
# Generated automatically by HLSFactory

open_project -reset correlation
set_top kernel_correlation
add_files correlation_kernel.c
add_files -tb tb_correlation.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "correlation HLS design"

exit
