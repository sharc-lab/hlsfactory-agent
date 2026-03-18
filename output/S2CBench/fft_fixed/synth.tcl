open_project /output/S2CBench/fft_fixed/project
set_top main
add_files -tb /output/S2CBench/fft_fixed/tb/*.cpp
add_files /output/S2CBench/fft_fixed/src/define.h
add_files /output/S2CBench/fft_fixed/src/fft.cpp
add_files /output/S2CBench/fft_fixed/src/fft.h
add_files /output/S2CBench/fft_fixed/src/main.cpp
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
