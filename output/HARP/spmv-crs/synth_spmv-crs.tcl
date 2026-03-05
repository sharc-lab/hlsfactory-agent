# HLS Synthesis Script for spmv-crs
# Generated automatically by HLSFactory

open_project -reset spmv-crs
set_top spmv
add_files spmv-crs_kernel.c
add_files -tb tb_spmv-crs.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "spmv-crs HLS design"

exit
