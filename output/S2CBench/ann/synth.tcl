open_project project
set_top ann::ann
add_files testbench.cpp layer.cpp ann_tb.cpp main.cpp ann.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
