open_solution -reset solution1
set_top kernel_jacobi_1d
add_files jacobi-1d_kernel.c
add_files -tb jacobi-1d_kernel_tb.cpp
open_solution solution1
csynth_design
exit
