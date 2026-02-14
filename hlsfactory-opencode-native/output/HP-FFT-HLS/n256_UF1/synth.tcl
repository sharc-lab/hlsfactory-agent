# Vivado HLS Synthesis Script for HP-FFT-HLS n256_UF1
# FFT Size: 256-point (8 stages)
# Variant: UF1

open_project n256_UF1
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
