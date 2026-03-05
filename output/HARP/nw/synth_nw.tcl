# HLS Synthesis Script for nw
# Generated automatically by HLSFactory

open_project -reset nw
set_top needwun
add_files nw_kernel.c
add_files -tb tb_nw.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "nw HLS design"

exit
