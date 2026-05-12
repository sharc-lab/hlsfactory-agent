set_top testbench
set_part xcu250-figd2104-2L-e
add_files testbench.cpp
add_files -tb testbench.cpp
open_solution "solution_1"
csynth_design
exit
