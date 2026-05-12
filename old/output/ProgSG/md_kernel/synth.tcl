open_solution -reset solution1
set_top md_kernel
add_files md_kernel.c
add_files -tb md_kernel_tb.cpp
open_solution solution1
csynth_design
exit
