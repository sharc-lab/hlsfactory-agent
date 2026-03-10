open_solution -flow_target vivado
set_top vgg16
add_files vgg16.cpp
add_files -tb vgg16_tb.cpp
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
