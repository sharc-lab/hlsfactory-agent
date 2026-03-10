open_solution -flow_target vivado
set_top mvt
add_files mvt.cpp
add_files -tb mvt_tb.cpp
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
