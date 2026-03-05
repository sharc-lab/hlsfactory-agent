# HLS Synthesis Script for seidel-2d
# Generated automatically by HLSFactory

open_project -reset seidel-2d
set_top kernel_seidel_2d
add_files seidel-2d_kernel.c
add_files -tb tb_seidel-2d.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "seidel-2d HLS design"

exit
