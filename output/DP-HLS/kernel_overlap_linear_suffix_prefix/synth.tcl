open_project kernel_overlap_linear_suffix_prefix
set_top unknown_top
add_files {kernel_overlap_linear_suffix_prefix.cpp}
add_files -tb {testbench.cpp}
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
