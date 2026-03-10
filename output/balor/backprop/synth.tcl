open_project backprop
set_top backprop
add_files backprop.cpp
add_files -tb backprop_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
