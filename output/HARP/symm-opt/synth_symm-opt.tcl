# HLS Synthesis Script for symm-opt
# Generated automatically by HLSFactory

open_project -reset symm-opt
set_top kernel_symm
add_files symm-opt_kernel.c
add_files -tb tb_symm-opt.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "symm-opt HLS design"

exit
