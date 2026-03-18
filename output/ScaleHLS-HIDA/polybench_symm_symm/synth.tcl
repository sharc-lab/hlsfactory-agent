open_project proj
set_top kernel_symm_node0
add_files symm.cpp
add_files -tb tb_polybench_symm_symm.cpp
open_solution "solution1" -flow_target "vivado"
set_part xcu250-figd2104-2L-e
csynth_design
exit
