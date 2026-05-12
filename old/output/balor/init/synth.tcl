open_project init
set_top init
add_files init.cpp
add_files -tb init_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
