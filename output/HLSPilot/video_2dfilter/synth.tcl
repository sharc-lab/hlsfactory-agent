open_project synth
set_top Write24BitBMP
set_part xcu250-figd2104-2L-e
add_files [glob *.cpp *.c *.h *.hpp]
add_files -tb testbench.cpp
open_solution "solution1"
csynth_design
exit
