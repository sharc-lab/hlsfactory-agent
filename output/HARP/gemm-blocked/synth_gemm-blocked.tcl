# HLS Synthesis Script for gemm-blocked
# Generated automatically by HLSFactory

open_project -reset gemm-blocked
set_top bbgemm
add_files gemm-blocked_kernel.c
add_files -tb tb_gemm-blocked.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "gemm-blocked HLS design"

exit
