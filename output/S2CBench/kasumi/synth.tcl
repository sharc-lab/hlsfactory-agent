open_project /output/S2CBench/kasumi/project
set_top kasumi
add_files -tb /output/S2CBench/kasumi/tb/*.cpp
add_files /output/S2CBench/kasumi/src/define.h
add_files /output/S2CBench/kasumi/src/kasumi.cpp
add_files /output/S2CBench/kasumi/src/kasumi.h
add_files /output/S2CBench/kasumi/src/main.cpp
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
