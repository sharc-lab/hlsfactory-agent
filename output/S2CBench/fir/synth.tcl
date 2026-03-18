open_project project
set_top void fir::fir_main ( void ) {
add_files fir.cpp tb_fir.cpp main.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
