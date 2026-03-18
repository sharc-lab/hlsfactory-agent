open_project project
set_top void kasumi::kasumi_run(){
add_files tb_kasumi.cpp kasumi.cpp main.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
