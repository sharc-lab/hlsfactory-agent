open_solution -reset solution1
set_top kernel_syr2k
add_files syr2k.c
add_files -tb syr2k_tb.cpp
open_solution solution1
csynth_design
exit
