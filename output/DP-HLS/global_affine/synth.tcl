open_project global_affine
set_top unknown_top
add_files {global_affine.cpp}
add_files -tb {testbench.cpp}
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
