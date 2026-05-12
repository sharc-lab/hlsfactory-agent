open_solution -reset solution1
set_top gemm
add_files gemm-ncubed.c
add_files -tb gemm-ncubed_tb.cpp
open_solution solution1
csynth_design
exit
