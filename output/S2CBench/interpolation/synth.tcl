open_project /output/S2CBench/interpolation/project
set_top filter_interp
add_files -tb /output/S2CBench/interpolation/tb/*.cpp
add_files /output/S2CBench/interpolation/src/define.h
add_files /output/S2CBench/interpolation/src/filter_interp.cpp
add_files /output/S2CBench/interpolation/src/filter_interp.h
add_files /output/S2CBench/interpolation/src/main.cpp
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
