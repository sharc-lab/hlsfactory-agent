open_project /output/S2CBench/uart/project
set_top uart
add_files -tb /output/S2CBench/uart/tb/*.cpp
add_files /output/S2CBench/uart/src/define.h
add_files /output/S2CBench/uart/src/main.cpp
add_files /output/S2CBench/uart/src/uart.cpp
add_files /output/S2CBench/uart/src/uart.h
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
