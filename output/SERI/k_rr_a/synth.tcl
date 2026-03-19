open_project k_rr_a_proj
set_top k_rr_a
add_files k_rr_a.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part {xcu250-figd2104-2L-e}
create_clock -period 5 -name default
csynth_design
exit
