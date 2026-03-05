# HLS Synthesis Script for fdtd-2d-large
# Generated automatically by HLSFactory

open_project -reset fdtd-2d-large
set_top kernel_fdtd_2d
add_files fdtd-2d-large_kernel.c
add_files -tb tb_fdtd-2d-large.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "fdtd-2d-large HLS design"

exit
