open_project k_find_bmax_proj
set_top /output/SERI/k_find_bmax/k_find_bmax.cpp:k_find_bmax
set_part xcu250-figd2104-2L-e
add_files {/output/SERI/k_find_bmax/*.cpp}
add_files -tb {/output/SERI/k_find_bmax/testbench.cpp}
open_solution "solution1"
csynth_design
exit
