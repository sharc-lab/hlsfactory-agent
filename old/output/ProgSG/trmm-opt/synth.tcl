open_solution -reset solution1
set_top kernel_trmm
add_files trmm-opt.c
add_files -tb trmm-opt_tb.cpp
open_solution solution1
csynth_design
exit
