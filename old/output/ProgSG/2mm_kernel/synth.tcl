open_solution -reset solution1
set_top kernel_2mm
add_files 2mm_kernel.c
add_files -tb 2mm_kernel_tb.cpp
open_solution solution1
csynth_design
exit
