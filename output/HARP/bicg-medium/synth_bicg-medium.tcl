# HLS Synthesis Script for bicg-medium
# Generated automatically by HLSFactory

open_project -reset bicg-medium
set_top kernel_bicg
add_files bicg-medium_kernel.c
add_files -tb tb_bicg-medium.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "bicg-medium HLS design"

exit
