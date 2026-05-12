set_top kernel-ultrascale
set_part xcu250-figd2104-2L-e
add_files kernel-ultrascale.cpp
add_files -tb testbench.cpp
open_solution "solution_1"
csynth_design
exit
