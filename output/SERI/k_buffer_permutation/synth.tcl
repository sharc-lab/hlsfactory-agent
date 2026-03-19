open_project k_buffer_permutation_proj
set_top k_buffer_permutation
add_files k_buffer_permutation.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part {xcu250-figd2104-2L-e}
create_clock -period 5 -name default
csynth_design
exit
