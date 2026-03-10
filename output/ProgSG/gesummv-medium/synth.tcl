open_solution -reset solution1
set_top kernel_gesummv
add_files gesummv-medium.c
add_files -tb gesummv-medium_tb.cpp
open_solution solution1
csynth_design
exit
