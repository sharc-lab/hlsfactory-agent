open_project dummy
set_top top
add_files dummy.cpp
add_files -tb tb_dummy.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
