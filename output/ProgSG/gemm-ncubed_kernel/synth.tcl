open_solution -reset solution1
set_top gemm
add_files gemm-ncubed_kernel.c
add_files -tb gemm-ncubed_kernel_tb.cpp
open_solution solution1
csynth_design
exit
