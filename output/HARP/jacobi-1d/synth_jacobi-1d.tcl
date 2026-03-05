# HLS Synthesis Script for jacobi-1d
# Generated automatically by HLSFactory

open_project -reset jacobi-1d
set_top kernel_jacobi_1d
add_files jacobi-1d_kernel.c
add_files -tb tb_jacobi-1d.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "jacobi-1d HLS design"

exit
