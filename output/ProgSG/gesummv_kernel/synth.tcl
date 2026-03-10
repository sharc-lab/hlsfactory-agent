open_solution -reset solution1
set_top kernel_gesummv
add_files gesummv_kernel.c
add_files -tb gesummv_kernel_tb.cpp
open_solution solution1
csynth_design
exit
