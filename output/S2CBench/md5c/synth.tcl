open_project /output/S2CBench/md5c/project
set_top md5c
add_files -tb /output/S2CBench/md5c/tb/*.cpp
add_files /output/S2CBench/md5c/src/define.h
add_files /output/S2CBench/md5c/src/main.cpp
add_files /output/S2CBench/md5c/src/md5c.cpp
add_files /output/S2CBench/md5c/src/md5c.h
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
