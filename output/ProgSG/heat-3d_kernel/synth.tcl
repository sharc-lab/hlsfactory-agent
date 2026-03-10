open_solution -reset solution1
set_top kernel_heat_3d
add_files heat-3d_kernel.c
add_files -tb heat-3d_kernel_tb.cpp
open_solution solution1
csynth_design
exit
