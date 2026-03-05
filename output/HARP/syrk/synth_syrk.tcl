# HLS Synthesis Script for syrk
# Generated automatically by HLSFactory

open_project -reset syrk
set_top kernel_syrk
add_files syrk_kernel.c
add_files -tb tb_syrk.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "syrk HLS design"

exit
