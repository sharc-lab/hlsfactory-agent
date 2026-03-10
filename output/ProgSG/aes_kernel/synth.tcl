open_solution -reset solution1
set_top aes_expandEncKey_1
add_files aes_kernel.c
add_files -tb aes_kernel_tb.cpp
open_solution solution1
csynth_design
exit
