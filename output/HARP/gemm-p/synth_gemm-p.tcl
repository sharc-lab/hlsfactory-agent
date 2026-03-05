# HLS Synthesis Script for gemm-p
# Generated automatically by HLSFactory

open_project -reset gemm-p
set_top kernel_gemm
add_files gemm-p_kernel.c
add_files -tb tb_gemm-p.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "gemm-p HLS design"

exit
