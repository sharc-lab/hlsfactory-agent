# HLS Synthesis Script for AutoNTT Design
open_project ntt_project
set_top NTT_kernel
add_files ntt_kernel.cpp
add_files ntt.h
open_solution "solution1" -flow_target vivado
set_part {xcvu9p-flga2104-2L-e}
create_clock -period 300MHz -name default
csynth_design
exit
