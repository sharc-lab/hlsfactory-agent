open_project project
set_top void ave8::ave8_main ( void ) {
add_files ave8.cpp tb_ave8.cpp main.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
