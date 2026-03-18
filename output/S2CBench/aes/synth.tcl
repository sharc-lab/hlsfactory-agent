open_project /output/S2CBench/aes/project
set_top 
add_files -tb /output/S2CBench/aes/tb/*.cpp
add_files /output/S2CBench/aes/src/*
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
