open_solution -reset solution1
set_top kernel_atax
add_files atax.c
add_files -tb atax_tb.cpp
open_solution solution1
csynth_design
exit
