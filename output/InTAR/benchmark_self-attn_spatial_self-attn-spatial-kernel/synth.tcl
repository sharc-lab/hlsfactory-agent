set_top self-attn-spatial-kernel
set_part xcu250-figd2104-2L-e
add_files self-attn-spatial-kernel.cpp
add_files -tb testbench.cpp
open_solution "solution_1"
csynth_design
exit
