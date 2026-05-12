open_project proj
set_top kernel_bicg_node0
add_files bicg.cpp
add_files -tb tb_polybench_bicg_bicg.cpp
open_solution "solution1" -flow_target "vivado"
set_part xcu250-figd2104-2L-e
csynth_design
exit
