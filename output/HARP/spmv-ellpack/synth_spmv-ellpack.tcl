# HLS Synthesis Script for spmv-ellpack
# Generated automatically by HLSFactory

open_project -reset spmv-ellpack
set_top ellpack
add_files spmv-ellpack_kernel.c
add_files -tb tb_spmv-ellpack.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "spmv-ellpack HLS design"

exit
