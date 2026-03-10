open_solution -reset solution1
set_top kernel_fdtd_2d
add_files fdtd-2d.c
add_files -tb fdtd-2d_tb.cpp
open_solution solution1
csynth_design
exit
