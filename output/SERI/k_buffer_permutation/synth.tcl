open_project k_buffer_permutation_proj
set_top /output/SERI/k_buffer_permutation/k_buffer_permutation.cpp:k_buffer_permutation
set_part xcu250-figd2104-2L-e
add_files {/output/SERI/k_buffer_permutation/*.cpp}
add_files -tb {/output/SERI/k_buffer_permutation/testbench.cpp}
open_solution "solution1"
csynth_design
exit
