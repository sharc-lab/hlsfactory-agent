open_project nn
set_top top_function
add_files [glob *.cpp *.c *.cc]
add_files -tb nn_tb.cpp
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
create_clock -period 10 -name default
csynth_design
exit
