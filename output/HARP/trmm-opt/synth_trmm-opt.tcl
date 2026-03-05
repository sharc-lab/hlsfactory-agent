# HLS Synthesis Script for trmm-opt
# Generated automatically by HLSFactory

open_project -reset trmm-opt
set_top kernel_trmm
add_files trmm-opt_kernel.c
add_files -tb tb_trmm-opt.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "trmm-opt HLS design"

exit
