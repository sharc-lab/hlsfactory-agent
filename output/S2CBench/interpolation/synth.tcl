open_project project
set_top void interp::run ( void ) {
add_files tb_interp.cpp filter_interp.cpp main.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
