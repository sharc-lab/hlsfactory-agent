set_top self-attn-sequential-kernel
set_part xcu250-figd2104-2L-e
add_files self-attn-sequential-kernel.cpp
add_files -tb testbench.cpp
open_solution "solution_1"
csynth_design
exit
