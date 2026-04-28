open_project matmul
set_top matmul
add_files matmul.cpp
add_files -tb testbench.cpp
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
