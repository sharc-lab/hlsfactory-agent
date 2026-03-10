open_solution -reset solution1
set_top stencil
add_files stencil_kernel.c
add_files -tb stencil_kernel_tb.cpp
open_solution solution1
csynth_design
exit
