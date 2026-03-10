open_solution -reset solution1
set_top kernel_bicg
add_files bicg-medium.c
add_files -tb bicg-medium_tb.cpp
open_solution solution1
csynth_design
exit
