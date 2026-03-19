open_project k_find_bmax_proj
set_top k_find_bmax
add_files k_find_bmax.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part {xcu250-figd2104-2L-e}
create_clock -period 5 -name default
csynth_design
exit
