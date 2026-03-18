open_project /output/S2CBench/disparity/project
set_top disparity
add_files -tb /output/S2CBench/disparity/tb/*.cpp
add_files /output/S2CBench/disparity/src/define.h
add_files /output/S2CBench/disparity/src/disparity.cpp
add_files /output/S2CBench/disparity/src/disparity.h
add_files /output/S2CBench/disparity/src/main.cpp
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
