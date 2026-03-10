open_solution -reset solution1
set_top kernel_gemver
add_files gemver.c
add_files -tb gemver_tb.cpp
open_solution solution1
csynth_design
exit
