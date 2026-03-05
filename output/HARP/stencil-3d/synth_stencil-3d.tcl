# HLS Synthesis Script for stencil-3d
# Generated automatically by HLSFactory

open_project -reset stencil-3d
set_top stencil3d
add_files stencil-3d_kernel.c
add_files -tb tb_stencil-3d.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "stencil-3d HLS design"

exit
