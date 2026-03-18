open_project /output/S2CBench/idct/project
set_top idct
add_files -tb /output/S2CBench/idct/tb/*.cpp
add_files /output/S2CBench/idct/src/define.h
add_files /output/S2CBench/idct/src/idct.cpp
add_files /output/S2CBench/idct/src/idct.h
add_files /output/S2CBench/idct/src/main.cpp
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
