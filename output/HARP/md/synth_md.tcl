# HLS Synthesis Script for md
# Generated automatically by HLSFactory

open_project -reset md
set_top md_kernel
add_files md_kernel.c
add_files -tb tb_md.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "md HLS design"

exit
