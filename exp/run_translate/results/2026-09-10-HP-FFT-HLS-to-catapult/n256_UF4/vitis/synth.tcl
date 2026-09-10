# TCL script for Vitis HLS synthesis
open_project prj_FFT
set_top FFT_TOP
add_files FFT.cpp
add_files -tb testbench.cpp
open_solution solution
set_part xczu9eg-ffvb1156-2-i
create_clock -period 4.0
csynth_design
exit
