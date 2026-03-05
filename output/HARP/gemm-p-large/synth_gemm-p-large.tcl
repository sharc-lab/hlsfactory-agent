# HLS Synthesis Script for gemm-p-large
# Generated automatically by HLSFactory

open_project -reset gemm-p-large
set_top kernel_gemm
add_files gemm-p-large_kernel.c
add_files -tb tb_gemm-p-large.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "gemm-p-large HLS design"

exit
