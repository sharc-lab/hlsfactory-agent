open_solution -reset solution1
set_top kernel_3mm
add_files 3mm.c
add_files -tb 3mm_tb.cpp
open_solution solution1
csynth_design
exit
