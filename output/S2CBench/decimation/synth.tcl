open_project /output/S2CBench/decimation/project
set_top filt_decim
add_files -tb /output/S2CBench/decimation/tb/*.cpp
add_files /output/S2CBench/decimation/src/define.h
add_files /output/S2CBench/decimation/src/filt_decim.cpp
add_files /output/S2CBench/decimation/src/filt_decim.h
add_files /output/S2CBench/decimation/src/main.cpp
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
