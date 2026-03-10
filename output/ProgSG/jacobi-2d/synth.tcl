open_solution -reset solution1
set_top kernel_jacobi_2d
add_files jacobi-2d.c
add_files -tb jacobi-2d_tb.cpp
open_solution solution1
csynth_design
exit
