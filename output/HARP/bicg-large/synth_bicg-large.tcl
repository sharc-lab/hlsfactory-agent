# HLS Synthesis Script for bicg-large
# Generated automatically by HLSFactory

open_project -reset bicg-large
set_top kernel_bicg
add_files bicg-large_kernel.c
add_files -tb tb_bicg-large.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "bicg-large HLS design"

exit
