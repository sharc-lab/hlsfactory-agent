open_solution -reset solution1
set_top kernel_symm
add_files symm-opt-medium.c
add_files -tb symm-opt-medium_tb.cpp
open_solution solution1
csynth_design
exit
