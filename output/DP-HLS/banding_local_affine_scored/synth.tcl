open_project banding_local_affine_scored
set_top unknown_top
add_files {banding_local_affine_scored.cpp}
add_files -tb {testbench.cpp}
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
