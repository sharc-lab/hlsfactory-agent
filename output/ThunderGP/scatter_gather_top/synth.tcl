open_project scatter_gather_top
set_top scatter_gather_top_top
add_files scatter_gather_top.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
