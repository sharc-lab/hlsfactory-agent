open_project tapa_qcf_proj
set_top t_preparation
add_files tapa_qcf.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part {xcu250-figd2104-2L-e}
create_clock -period 5 -name default
csynth_design
exit
