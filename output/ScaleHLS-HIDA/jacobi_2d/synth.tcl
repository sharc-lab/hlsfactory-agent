open_solution -flow_target vivado
set_top jacobi_2d
add_files jacobi_2d.cpp
add_files -tb jacobi_2d_tb.cpp
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
