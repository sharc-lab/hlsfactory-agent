open_project matrix_multiplication
set_top MatrixMultiplicationKernel
add_files Top.cpp
add_files Memory.cpp
add_files Compute.cpp
add_files -tb testbench.cpp
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
