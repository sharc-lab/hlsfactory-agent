open_solution -flow_target vivado
set_top 2mm
add_files 2mm.cpp
add_files -tb 2mm_tb.cpp
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
