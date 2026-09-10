open_project mac_proj
set_top mac
add_files mac.cpp
add_files mac.h
add_files -tb testbench.cpp
open_solution solution1
set_part xczu9eg-ffvb1156-2-i
create_clock -period 5
csynth_design
exit
