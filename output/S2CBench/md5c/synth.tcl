open_project project
set_top int sc_main(int argc, char** argv)
add_files md5c.cpp tb_md5c.cpp main.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
