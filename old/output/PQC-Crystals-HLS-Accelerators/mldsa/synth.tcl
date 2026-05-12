open_project mldsa_proj
set_top mldsa_accelerator
add_files k_dsa.cpp
add_files kernel.hpp
add_files -tb testbench.cpp
open_solution "solution1" -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
