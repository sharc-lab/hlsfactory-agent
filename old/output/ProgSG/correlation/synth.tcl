open_solution -reset solution1
set_top kernel_correlation
add_files correlation.c
add_files -tb correlation_tb.cpp
open_solution solution1
csynth_design
exit
