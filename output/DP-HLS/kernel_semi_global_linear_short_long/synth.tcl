open_project kernel_semi_global_linear_short_long
set_top unknown_top
add_files {kernel_semi_global_linear_short_long.cpp}
add_files -tb {testbench.cpp}
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
