open_solution -reset solution1
set_top md_kernel
add_files md.c
add_files -tb md_tb.cpp
open_solution solution1
csynth_design
exit
