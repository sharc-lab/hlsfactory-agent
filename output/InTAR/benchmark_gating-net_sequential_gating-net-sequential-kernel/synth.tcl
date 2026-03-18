set_top gating-net-sequential-kernel
set_part xcu250-figd2104-2L-e
add_files gating-net-sequential-kernel.cpp
add_files -tb testbench.cpp
open_solution "solution_1"
csynth_design
exit
