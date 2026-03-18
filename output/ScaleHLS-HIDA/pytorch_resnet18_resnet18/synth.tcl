open_project proj
set_top forward_node1
add_files resnet18.cpp
add_files -tb tb_pytorch_resnet18_resnet18.cpp
open_solution "solution1" -flow_target "vivado"
set_part xcu250-figd2104-2L-e
csynth_design
exit
