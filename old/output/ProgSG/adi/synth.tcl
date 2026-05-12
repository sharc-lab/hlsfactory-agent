open_solution -reset solution1
set_top kernel_adi
add_files adi.c
add_files -tb adi_tb.cpp
open_solution solution1
csynth_design
exit
