open_project k_rr_a_proj
set_top /output/SERI/k_rr_a/k_rr_a.cpp:k_rr_a
set_part xcu250-figd2104-2L-e
add_files {/output/SERI/k_rr_a/*.cpp}
add_files -tb {/output/SERI/k_rr_a/testbench.cpp}
open_solution "solution1"
csynth_design
exit
