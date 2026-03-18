open_project /output/S2CBench/sobel/project
set_top main
add_files -tb /output/S2CBench/sobel/tb/*.cpp
add_files /output/S2CBench/sobel/src/define.h
add_files /output/S2CBench/sobel/src/main.cpp
add_files /output/S2CBench/sobel/src/sobel.cpp
add_files /output/S2CBench/sobel/src/sobel.h
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
