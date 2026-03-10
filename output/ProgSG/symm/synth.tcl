open_solution -reset solution1
set_top kernel_symm
add_files symm.c
add_files -tb symm_tb.cpp
open_solution solution1
csynth_design
exit
