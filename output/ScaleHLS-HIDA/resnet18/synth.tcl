open_solution -flow_target vivado
set_top resnet18
add_files resnet18.cpp
add_files -tb resnet18_tb.cpp
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
