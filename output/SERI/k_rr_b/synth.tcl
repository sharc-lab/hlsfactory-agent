open_project k_rr_b_proj
set_top /output/SERI/k_rr_b/k_rr_b.cpp:k_rr_b
set_part xcu250-figd2104-2L-e
add_files {/output/SERI/k_rr_b/*.cpp}
add_files -tb {/output/SERI/k_rr_b/testbench.cpp}
open_solution "solution1"
csynth_design
exit
