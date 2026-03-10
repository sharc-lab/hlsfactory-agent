open_solution -flow_target vivado
set_top syr2k
add_files syr2k.cpp
add_files -tb syr2k_tb.cpp
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
