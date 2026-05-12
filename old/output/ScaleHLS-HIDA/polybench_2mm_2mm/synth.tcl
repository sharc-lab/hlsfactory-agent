open_project proj
set_top kernel_2mm_node0
add_files 2mm.cpp
add_files -tb tb_polybench_2mm_2mm.cpp
open_solution "solution1" -flow_target "vivado"
set_part xcu250-figd2104-2L-e
csynth_design
exit
