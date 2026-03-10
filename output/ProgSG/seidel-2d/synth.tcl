open_solution -reset solution1
set_top kernel_seidel_2d
add_files seidel-2d.c
add_files -tb seidel-2d_tb.cpp
open_solution solution1
csynth_design
exit
