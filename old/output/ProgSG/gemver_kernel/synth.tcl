open_solution -reset solution1
set_top kernel_gemver
add_files gemver_kernel.c
add_files -tb gemver_kernel_tb.cpp
open_solution solution1
csynth_design
exit
