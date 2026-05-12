open_solution -reset solution1
set_top bbgemm
add_files gemm-blocked_kernel.c
add_files -tb gemm-blocked_kernel_tb.cpp
open_solution solution1
csynth_design
exit
