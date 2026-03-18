open_project project
set_top void cholesky::cholesky_main ( void ) {
add_files tb_cholesky.cpp main.cpp cholesky.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
