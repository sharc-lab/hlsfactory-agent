# HLS Synthesis Script for gemm-ncubed
# Generated automatically by HLSFactory

open_project -reset gemm-ncubed
set_top gemm
add_files gemm-ncubed_kernel.c
add_files -tb tb_gemm-ncubed.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "gemm-ncubed HLS design"

exit
