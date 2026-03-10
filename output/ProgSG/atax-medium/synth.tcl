open_solution -reset solution1
set_top kernel_atax
add_files atax-medium.c
add_files -tb atax-medium_tb.cpp
open_solution solution1
csynth_design
exit
