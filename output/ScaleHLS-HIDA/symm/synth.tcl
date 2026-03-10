open_solution -flow_target vivado
set_top symm
add_files symm.cpp
add_files -tb symm_tb.cpp
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
