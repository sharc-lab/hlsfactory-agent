open_solution -reset solution1
set_top kernel_bicg
add_files bicg_kernel.c
add_files -tb bicg_kernel_tb.cpp
open_solution solution1
csynth_design
exit
