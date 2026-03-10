open_solution -reset solution1
set_top kernel_gemm
add_files gemm-p-large.c
add_files -tb gemm-p-large_tb.cpp
open_solution solution1
csynth_design
exit
