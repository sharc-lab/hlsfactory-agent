open_project md_kernel
set_top md_kernel
add_files md_kernel.cpp
add_files -tb md_kernel_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
