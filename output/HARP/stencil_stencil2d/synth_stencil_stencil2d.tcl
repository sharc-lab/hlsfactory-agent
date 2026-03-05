# HLS Synthesis Script for stencil_stencil2d
# Generated automatically by HLSFactory

open_project -reset stencil_stencil2d
set_top stencil
add_files stencil_stencil2d_kernel.c
add_files -tb tb_stencil_stencil2d.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "stencil_stencil2d HLS design"

exit
