open_project proj
set_top kernel_3mm_node0
add_files 3mm.cpp
add_files -tb tb_polybench_3mm_3mm.cpp
open_solution "solution1" -flow_target "vivado"
set_part xcu250-figd2104-2L-e
csynth_design
exit
