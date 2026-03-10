open_solution -reset solution1
set_top kernel_gemver
add_files gemver-medium.c
add_files -tb gemver-medium_tb.cpp
open_solution solution1
csynth_design
exit
