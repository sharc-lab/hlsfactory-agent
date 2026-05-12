open_project proj
set_top kernel_correlation_node0
add_files correlation.cpp
add_files -tb tb_polybench_correlation_correlation.cpp
open_solution "solution1" -flow_target "vivado"
set_part xcu250-figd2104-2L-e
csynth_design
exit
