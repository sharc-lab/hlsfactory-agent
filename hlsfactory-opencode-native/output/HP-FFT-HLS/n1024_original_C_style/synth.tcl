# Vivado HLS Synthesis Script for HP-FFT-HLS n1024_original_C_style
# FFT Size: 1024-point (10 stages)
# Variant: original_C_style

open_project n1024_original_C_style
set_top FFT_TOP
add_files FFT.cpp -cflags "-I."
add_files -tb testbench.cpp -cflags "-I."
open_solution "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 5 -name default

# HLS configurations for optimization
csim_design
csynth_design
cosim_design -O
export_design -format ip_catalog
close_project
exit
