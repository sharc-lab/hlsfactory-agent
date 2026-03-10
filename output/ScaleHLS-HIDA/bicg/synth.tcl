open_solution -flow_target vivado
set_top bicg
add_files bicg.cpp
add_files -tb bicg_tb.cpp
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
