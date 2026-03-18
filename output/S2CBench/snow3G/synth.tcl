open_project project
set_top int sc_main(int argc, char** argv)
add_files tb_snow_3G.cpp snow_3G.cpp main.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
