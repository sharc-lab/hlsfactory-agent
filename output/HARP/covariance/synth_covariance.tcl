# HLS Synthesis Script for covariance
# Generated automatically by HLSFactory

open_project -reset covariance
set_top kernel_covariance
add_files covariance_kernel.c
add_files -tb tb_covariance.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "covariance HLS design"

exit
