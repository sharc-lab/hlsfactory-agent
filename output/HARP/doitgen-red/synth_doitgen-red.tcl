# HLS Synthesis Script for doitgen-red
# Generated automatically by HLSFactory

open_project -reset doitgen-red
set_top kernel_doitgen
add_files doitgen-red_kernel.c
add_files -tb tb_doitgen-red.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "doitgen-red HLS design"

exit
