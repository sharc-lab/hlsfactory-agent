open_project pe
set_top e
add_files pe.cpp
add_files -tb tb.cpp
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
create_clock -period 3.33 -name default
csynth_design
exit
