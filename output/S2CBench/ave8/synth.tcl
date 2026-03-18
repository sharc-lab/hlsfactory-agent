open_project /output/S2CBench/ave8/project
set_top ave8
add_files -tb /output/S2CBench/ave8/tb/*.cpp
add_files /output/S2CBench/ave8/src/ave8.cpp
add_files /output/S2CBench/ave8/src/ave8.h
add_files /output/S2CBench/ave8/src/define.h
add_files /output/S2CBench/ave8/src/main.cpp
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
