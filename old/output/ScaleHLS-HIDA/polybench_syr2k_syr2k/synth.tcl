open_project proj
set_top kernel_syr2k_node1
add_files syr2k.cpp
add_files -tb tb_polybench_syr2k_syr2k.cpp
open_solution "solution1" -flow_target "vivado"
set_part xcu250-figd2104-2L-e
csynth_design
exit
