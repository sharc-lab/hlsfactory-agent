open_project proj
set_top kernel_atax_node1
add_files atax.cpp
add_files -tb tb_polybench_atax_atax.cpp
open_solution "solution1" -flow_target "vivado"
set_part xcu250-figd2104-2L-e
csynth_design
exit
