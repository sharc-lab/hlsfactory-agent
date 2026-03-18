open_project /output/S2CBench/qsort/project
set_top qsort
add_files -tb /output/S2CBench/qsort/tb/*.cpp
add_files /output/S2CBench/qsort/src/define.h
add_files /output/S2CBench/qsort/src/main.cpp
add_files /output/S2CBench/qsort/src/qsort.cpp
add_files /output/S2CBench/qsort/src/qsort.h
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
