open_solution -reset solution1
set_top bbgemm
add_files gemm-blocked.c
add_files -tb gemm-blocked_tb.cpp
open_solution solution1
csynth_design
exit
