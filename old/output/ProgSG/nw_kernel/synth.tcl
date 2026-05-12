open_solution -reset solution1
set_top needwun
add_files nw_kernel.c
add_files -tb nw_kernel_tb.cpp
open_solution solution1
csynth_design
exit
