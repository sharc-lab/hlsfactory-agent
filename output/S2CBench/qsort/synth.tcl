open_project project
set_top int sc_main(int argc, char** argv)
add_files qsort.cpp main.cpp tb_qsort.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
