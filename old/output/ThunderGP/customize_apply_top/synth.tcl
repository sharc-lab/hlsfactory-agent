open_project customize_apply_top
set_top vertexApply
add_files customize_apply_top.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
