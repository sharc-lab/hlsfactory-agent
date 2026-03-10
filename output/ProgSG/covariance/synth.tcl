open_solution -reset solution1
set_top kernel_covariance
add_files covariance.c
add_files -tb covariance_tb.cpp
open_solution solution1
csynth_design
exit
