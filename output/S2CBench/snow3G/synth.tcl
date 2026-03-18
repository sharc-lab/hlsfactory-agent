open_project /output/S2CBench/snow3G/project
set_top snow_3G
add_files -tb /output/S2CBench/snow3G/tb/*.cpp
add_files /output/S2CBench/snow3G/src/define.h
add_files /output/S2CBench/snow3G/src/main.cpp
add_files /output/S2CBench/snow3G/src/snow_3G.cpp
add_files /output/S2CBench/snow3G/src/snow_3G.h
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
