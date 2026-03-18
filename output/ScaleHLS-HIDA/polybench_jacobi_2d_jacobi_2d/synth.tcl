open_project proj
set_top kernel_jacobi_2d_node1
add_files jacobi_2d.cpp
add_files -tb tb_polybench_jacobi_2d_jacobi_2d.cpp
open_solution "solution1" -flow_target "vivado"
set_part xcu250-figd2104-2L-e
csynth_design
exit
