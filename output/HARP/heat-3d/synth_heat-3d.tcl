# HLS Synthesis Script for heat-3d
# Generated automatically by HLSFactory

open_project -reset heat-3d
set_top kernel_heat_3d
add_files heat-3d_kernel.c
add_files -tb tb_heat-3d.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "heat-3d HLS design"

exit
