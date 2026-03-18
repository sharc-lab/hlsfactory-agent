open_project /output/S2CBench/fir/project
set_top fir
add_files -tb /output/S2CBench/fir/tb/*.cpp
add_files /output/S2CBench/fir/src/define.h
add_files /output/S2CBench/fir/src/fir.cpp
add_files /output/S2CBench/fir/src/fir.h
add_files /output/S2CBench/fir/src/main.cpp
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
