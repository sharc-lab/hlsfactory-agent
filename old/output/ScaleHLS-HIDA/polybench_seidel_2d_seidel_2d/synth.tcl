open_project proj
set_top kernel_seidel_2d_node0
add_files seidel_2d.cpp
add_files -tb tb_polybench_seidel_2d_seidel_2d.cpp
open_solution "solution1" -flow_target "vivado"
set_part xcu250-figd2104-2L-e
csynth_design
exit
