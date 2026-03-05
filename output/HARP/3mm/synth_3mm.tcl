# HLS Synthesis Script for 3mm
# Generated automatically by HLSFactory

open_project -reset 3mm
set_top kernel_3mm
add_files 3mm_kernel.c
add_files -tb tb_3mm.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "3mm HLS design"

exit
