open_project /output/S2CBench/cholesky/project
set_top main
add_files -tb /output/S2CBench/cholesky/tb/*.cpp
add_files /output/S2CBench/cholesky/src/cholesky.cpp
add_files /output/S2CBench/cholesky/src/cholesky.h
add_files /output/S2CBench/cholesky/src/define.h
add_files /output/S2CBench/cholesky/src/main.cpp
add_files /output/S2CBench/cholesky/src/square_root_lut.h
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
