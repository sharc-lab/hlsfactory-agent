# HLS Synthesis Script for mvt
# Generated automatically by HLSFactory

open_project -reset mvt
set_top kernel_mvt
add_files mvt_kernel.c
add_files -tb tb_mvt.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "mvt HLS design"

exit
