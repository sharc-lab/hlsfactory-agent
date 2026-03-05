# HLS Synthesis Script for 2mm
# Generated automatically by HLSFactory

open_project -reset 2mm
set_top kernel_2mm
add_files 2mm_kernel.c
add_files -tb tb_2mm.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "2mm HLS design"

exit
