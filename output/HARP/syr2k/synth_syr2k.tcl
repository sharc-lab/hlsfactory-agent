# HLS Synthesis Script for syr2k
# Generated automatically by HLSFactory

open_project -reset syr2k
set_top kernel_syr2k
add_files syr2k_kernel.c
add_files -tb tb_syr2k.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "syr2k HLS design"

exit
