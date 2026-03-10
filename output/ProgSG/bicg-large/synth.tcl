open_solution -reset solution1
set_top kernel_bicg
add_files bicg-large.c
add_files -tb bicg-large_tb.cpp
open_solution solution1
csynth_design
exit
